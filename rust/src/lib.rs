extern crate docopt;
extern crate libc;
#[macro_use]
extern crate serde_derive;

use std::fs::File;
use std::io;
use std::io::prelude::*;
use std::os::unix::io::AsRawFd;
use std::path::Path;
use std::process::{Command, Output};

pub mod cli;
use cli::Config;

struct Log {}

pub struct Bench {
    target: File,
    buffer: Vec<u8>,
    logs: Vec<Log>,
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

pub fn setup(config: &Config) -> BenchResult<Bench> {
    let target_path = Path::new(&config.workdir).join("benchmark-io.bin");
    let mut target = File::open(target_path)?;

    print!("Preparing target file ... ");
    io::stdout().flush().unwrap();
    setup_target(&mut target, 0)?;
    println!("done.");

    print!("Clearing page cache ... ");
    io::stdout().flush().unwrap();
    setup_clear_cache()?;
    println!("done.");

    Ok(Bench {
        target,
        buffer: Vec::with_capacity(config.bs),
        logs: Vec::with_capacity(config.count + 1),
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

pub fn run(config: &Config, bench: &mut Bench) -> BenchResult<()> {
    Ok(())
}

pub fn teardown(config: &Config, bench: &mut Bench) -> BenchResult<()> {
    Ok(())
}
