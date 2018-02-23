use docopt::Docopt;
use std::fmt;
use std::path::{Path, PathBuf};

const USAGE: &'static str = "
Simple file I/O benchmark.

Usage:
  benchmark-io-rust [options] (--rread | --rwrite | --sread | --swrite) WORKDIR
  benchmark-io-rust (-h | --help)

Options:
  -h --help             Show this screen.
  -b BYTES, --bs=BYTES  Block size of each I/O [default: 4096]
  -c N, --count=N       Total number of I/Os [default: 100000].
  --filesize-gib=GIB    Target file size in GiB [default: 1].
  --rread               Issue random reads.
  --rwrite              Issue random writes.
  --sread               Issue sequential reads.
  --swrite              Issue sequential writes.
  --no-clear-cache      Skip clearing page cache in setup.
  --no-write-log        Skip writing I/O log.
";

#[allow(non_snake_case)]
#[derive(Debug, Deserialize)]
struct Args {
    arg_WORKDIR: String,
    flag_bs: u64,
    flag_count: u64,
    flag_filesize_gib: u64,
    flag_rread: bool,
    flag_rwrite: bool,
    flag_sread: bool,
    flag_swrite: bool,
    flag_no_clear_cache: bool,
    flag_no_write_log: bool,
}

#[derive(Debug)]
pub enum Io {
    RandRead,
    RandWrite,
    SeqRead,
    SeqWrite,
}

impl Io {
    pub fn to_abbrev(&self) -> &str {
        match self {
            &Io::RandRead => "RR",
            &Io::RandWrite => "RW",
            &Io::SeqRead => "SR",
            &Io::SeqWrite => "SW",
        }
    }
}

impl fmt::Display for Io {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let s = match self {
            &Io::RandRead => "Random read",
            &Io::RandWrite => "Random write",
            &Io::SeqRead => "Sequential read",
            &Io::SeqWrite => "Sequential write",
        };
        write!(f, "{}", s)
    }
}

#[derive(Debug)]
pub struct Config {
    pub program: &'static str,
    pub workdir: String,
    pub io_type: Io,
    pub bs: u64,
    pub count: u64,
    pub filesize: u64,
    pub clear_cache: bool,
    pub write_log: bool,
}

impl Config {
    fn from(args: Args) -> Config {
        let io_type = match (
            args.flag_rread,
            args.flag_rwrite,
            args.flag_sread,
            args.flag_swrite,
        ) {
            (true, _, _, _) => Io::RandRead,
            (_, true, _, _) => Io::RandWrite,
            (_, _, true, _) => Io::SeqRead,
            (_, _, _, true) => Io::SeqWrite,
            (_, _, _, _) => unreachable!(),
        };
        Config {
            program: "benchmark-io-rust",
            workdir: args.arg_WORKDIR,
            io_type,
            bs: args.flag_bs,
            count: args.flag_count,
            filesize: args.flag_filesize_gib * (1 << 30),
            clear_cache: !args.flag_no_clear_cache,
            write_log: !args.flag_no_write_log,
        }
    }

    pub fn hello(&self) {
        println!(
            "\
{}; Simple file I/O benchmark.

Config:
  Working directory  {}
  I/O type           {}
  Block size [byte]  {}
  Count              {}
  File size [GiB]    {}
",
            self.program,
            self.workdir,
            self.io_type,
            self.bs,
            self.count,
            self.filesize >> 30
        )
    }

    pub fn path_for(&self, filename: &str) -> PathBuf {
        Path::new(&self.workdir).join(filename)
    }
}

pub fn parse_args() -> Config {
    let args: Args = Docopt::new(USAGE)
        .and_then(|d| d.deserialize())
        .unwrap_or_else(|e| e.exit());
    Config::from(args)
}
