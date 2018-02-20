extern crate chrono;
extern crate docopt;
extern crate libc;
extern crate rand;
#[macro_use]
extern crate serde_derive;

use rand::Rng;
use std::fs::{File, OpenOptions};
use std::io;
use std::io::prelude::*;
use std::os::unix::io::AsRawFd;
use std::process::{Command, Output};
use std::time::{Duration, Instant};

trait ToSec {
    fn to_sec(&self) -> f64;
}

impl ToSec for Duration {
    fn to_sec(&self) -> f64 {
        self.as_secs() as f64 + self.subsec_nanos() as f64 / 1e9
    }
}

pub mod cli;
use cli::{Config, Io};

struct Log {
    elapsed: Duration,
    offset: u64,
    complete_bs: usize,
}

pub struct Bench {
    config: Config,
    rng: rand::ThreadRng,
    target: File,
    buffer: Vec<u8>,
    logs: Vec<Log>,
    start: Option<Instant>,
}

impl Bench {
    fn seek(&mut self, i: &u64) -> BenchResult<u64> {
        let offset;
        match self.config.io_type {
            Io::RandRead | Io::RandWrite => {
                offset = self.config.bs * self.rng.gen_range::<u64>(0, self.config.count);
                self.target.seek(io::SeekFrom::Start(offset))?;
            }
            Io::SeqRead | Io::SeqWrite => {
                offset = self.config.bs * i;
            }
        }
        Ok(offset)
    }

    fn issue_io(&mut self) -> BenchResult<usize> {
        let complete_bs;
        match self.config.io_type {
            Io::RandRead | Io::SeqRead => {
                complete_bs = self.target.read(self.buffer.as_mut_slice())?;
            }
            Io::RandWrite | Io::SeqWrite => {
                complete_bs = self.target.write(&self.buffer)?;
                self.target.sync_data()?;
            }
        }
        Ok(complete_bs)
    }

    fn dump_logs(&self, filename: &str) -> BenchResult<()> {
        let path = self.config.path_for(filename);
        print!("Writing log to {} ... ", path.display());
        io::stdout().flush().unwrap();

        let outfile = File::create(path)?;
        let mut writer = io::BufWriter::new(outfile);

        write!(writer, "elapsed_time,io_type,offset,issue_bs,complete_s\n")?;
        for log in &self.logs {
            write!(
                writer,
                "{}.{:09},{},{},{},{}\n",
                log.elapsed.as_secs(),
                log.elapsed.subsec_nanos(),
                self.config.io_type.to_abbrev(),
                log.offset,
                self.config.bs,
                log.complete_bs
            )?;
        }

        println!("done.");
        Ok(())
    }

    fn show_summary(&self) {
        let elapsed = match self.logs.last() {
            Some(log) => log.elapsed,
            None => return,
        };
        let elapsed_s = elapsed.to_sec();
        let total_complete_bs: usize = self.logs.iter().map(|ref log| log.complete_bs).sum();
        let throughout_mib = total_complete_bs as f64 / (1 << 20) as f64 / elapsed_s;
        let iops = self.config.count as f64 / elapsed_s;
        let mean_latency_ms = elapsed_s as f64 / self.config.count as f64 * 1e3;
        println!(
            "\
Sumary:
  Elapsed time  {:.3} [s]
  Throughput    {:.3} [MiB/s]
  IOPS          {:.3}
  Mean latency  {:.3} [ms]
",
            elapsed_s, throughout_mib, iops, mean_latency_ms
        );
    }
}

#[derive(Debug)]
pub enum BenchError {
    Command(Output),
    Io(io::Error),
    Unsafe(&'static str),
}

impl From<io::Error> for BenchError {
    fn from(err: io::Error) -> BenchError {
        BenchError::Io(err)
    }
}

pub type BenchResult<T> = Result<T, BenchError>;

pub fn setup(config: Config) -> BenchResult<Bench> {
    print!("Preparing target file ... ");
    io::stdout().flush().unwrap();

    let mut target = OpenOptions::new()
        .read(true)
        .write(true)
        .create(true)
        .open(config.path_for("benchmark-io.bin"))?;
    setup_target(&mut target, config.filesize as u64)?;

    println!("done.");

    print!("Clearing page cache ... ");
    io::stdout().flush().unwrap();
    setup_clear_cache()?;
    println!("done.");

    Ok(Bench {
        rng: rand::thread_rng(),
        target,
        buffer: vec![0; config.bs as usize],
        logs: Vec::with_capacity(config.count as usize),
        config,
        start: None,
    })
}

fn setup_target(target: &mut File, size: u64) -> BenchResult<()> {
    let actual_size = target.seek(io::SeekFrom::End(0))?;
    if actual_size < size {
        let ret = unsafe { libc::ftruncate(target.as_raw_fd(), size as libc::off_t) };
        if ret == -1 {
            return Err(BenchError::Unsafe("libc::ftruncate() failed"));
        }
    }
    target.seek(io::SeekFrom::Start(0))?;
    Ok(())
}

fn setup_clear_cache() -> BenchResult<()> {
    let output = Command::new("sudo")
        .args(&["sysctl", "-w", "vm.drop_caches=3"])
        .output()?;
    if !output.status.success() {
        return Err(BenchError::Command(output));
    }
    Ok(())
}

pub fn run(bench: &mut Bench) -> BenchResult<()> {
    print!("\nRunning benchmark ... ");
    io::stdout().flush().unwrap();

    bench.start = Some(Instant::now());
    for i in 0..bench.config.count {
        let offset = bench.seek(&i)?;
        let complete_bs = bench.issue_io()?;
        bench.logs.push(Log {
            elapsed: bench.start.unwrap().elapsed(),
            offset,
            complete_bs,
        });
    }

    println!("done.");
    Ok(())
}

pub fn teardown(bench: &mut Bench) -> BenchResult<()> {
    let now = chrono::Local::now();
    bench.dump_logs(&now.format("benchmark-io_%F-%H-%M-%S.log").to_string())?;

    bench.show_summary();
    Ok(())
}
