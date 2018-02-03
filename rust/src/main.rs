extern crate benchmark_io_rust as bench;
use bench::cli;

fn main() {
    let config = cli::parse_args();
    config.hello();
}
