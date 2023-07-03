mod types;

extern "C" {
    fn main_c() -> u32;
}

fn main() {
    unsafe { main_c(); }
    
    println!("Hello, world!");
}
