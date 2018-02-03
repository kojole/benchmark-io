use std::fmt;
use docopt::Docopt;

const USAGE: &'static str = "
Simple file I/O benchmark.

Usage:
  benchmark-io-rust [options] (--rread | --rwrite | --sread | --swrite) <workdir>
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
";

#[derive(Debug, Deserialize)]
struct Args {
    arg_workdir: String,
    flag_bs: isize,
    flag_count: isize,
    flag_filesize_gib: isize,
    flag_rread: bool,
    flag_rwrite: bool,
    flag_sread: bool,
    flag_swrite: bool,
}

#[derive(Debug)]
pub enum IO {
    RandRead,
    RandWrite,
    SeqRead,
    SeqWrite,
}

impl fmt::Display for IO {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let s = match self {
            &IO::RandRead => "Random read",
            &IO::RandWrite => "Random write",
            &IO::SeqRead => "Sequential read",
            &IO::SeqWrite => "Sequential write",
        };
        write!(f, "{}", s)
    }
}

#[derive(Debug)]
pub struct Config {
    pub program: &'static str,
    pub workdir: String,
    pub io_type: IO,
    pub bs: isize,
    pub count: isize,
    pub filesize: isize,
}

impl Config {
    fn from(args: Args) -> Config {
        let io_type = match (
            args.flag_rread,
            args.flag_rwrite,
            args.flag_sread,
            args.flag_swrite,
        ) {
            (true, _, _, _) => IO::RandRead,
            (_, true, _, _) => IO::RandWrite,
            (_, _, true, _) => IO::SeqRead,
            (_, _, _, true) => IO::SeqWrite,
            (_, _, _, _) => unreachable!(),
        };
        Config {
            program: "benchmark-io-rust",
            workdir: args.arg_workdir,
            io_type,
            bs: args.flag_bs,
            count: args.flag_count,
            filesize: args.flag_filesize_gib * (1 << 30),
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
}

pub fn parse_args() -> Config {
    let args: Args = Docopt::new(USAGE)
        .and_then(|d| d.deserialize())
        .unwrap_or_else(|e| e.exit());
    Config::from(args)
}
