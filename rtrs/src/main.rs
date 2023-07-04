mod types;

extern "C" {
    fn test() -> u32;
}

fn main() {
    unsafe { assert!(test() == 1); }
}
