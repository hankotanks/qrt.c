use std::fs;

fn main() {
    match project_root::get_project_root() {
        Ok(crate_root) => match crate_root.parent() {
            Some(project_root) => {
                let source_dir = project_root.join("src").join("rt.c");
                println!("cargo:rerun-if-changed={:?}", source_dir);

                let include_dir = project_root.join("include");
                match fs::read_dir(&include_dir) {
                    Ok(include_dir_read) => for entry in include_dir_read {
                        if let Ok(entry) = entry {
                            println!("cargo:rerun-if-changed={:?}", entry.path())
                        }
                    },
                    Err(_) => {
                        eprintln!("Build Error: Failed to read include directory: {:?}", include_dir);
                    }
                }

                cc::Build::new()
                    .file(source_dir)
                    .include(include_dir)
                    .compile("rt");

                println!("Rebuilt `rt.c` library...");
            },
            None => eprintln!("Build Error: Could not find project root.")
        },
        Err(_) => eprintln!("Build Error: Could not find project root.")
    };
}