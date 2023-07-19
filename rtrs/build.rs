use std::fs;

use anyhow::anyhow;

fn main() -> anyhow::Result<()> {
    let crate_path = project_root::get_project_root()?;
    let project_path = crate_path.parent().ok_or(
        anyhow!("Build Error: Broken directory structure."))?;

    let stub_path = crate_path.join("interface").join("stub.c");

    let include_dir = project_path.join("include");
    for entry in fs::read_dir(&include_dir)? {
        match entry?.path().into_os_string().into_string() {
            Ok(entry_path_string) => println!("cargo:rerun-if-changed={}", entry_path_string),
            Err(_) => return Err(
                anyhow!("Build Error: Path to header files isn't UTF-8 encoded."))
        }
    }

    cc::Build::new()
        .file(&stub_path)
        .include(include_dir)
        .compile("rt");

    Ok(())
}