use std::ffi::CString;
use std::os::raw::{c_char, c_int};

use tokio::sync::oneshot::channel;

pub fn rust_main(cmain: unsafe extern "C" fn(c_int, *const *mut c_char)) {
    cjdnslog::install();
    sodiumoxide::init().unwrap();
    if std::env::var("RUST_BACKTRACE").is_err() {
        std::env::set_var("RUST_BACKTRACE", "full");
    }

    tokio::runtime::Builder::new_multi_thread()
        .worker_threads(8)
        .enable_all()
        .build()
        .unwrap()
        .block_on(async {
            let default_panic = std::panic::take_hook();
            std::panic::set_hook(Box::new(move |info| {
                default_panic(info);
                std::process::abort();
            }));
            let (done_send, done_recv) = channel::<()>();
            tokio::task::spawn_blocking(move ||{
                let _done_send = done_send;
                let c_args = std::env::args()
                    .map(|arg| CString::new(arg).unwrap().into_raw())
                    .collect::<Vec<_>>();
                unsafe { cmain(c_args.len() as c_int, c_args.as_ptr()) }
            });
            let _ = done_recv.await;
        })
}

#[macro_export]
macro_rules! c_main {
    ($cmain:ident) => {
        use std::os::raw::{c_char, c_int};
        extern "C" {
            fn $cmain(argc: c_int, argv: *const *mut c_char);
        }
        fn main() {
            cjdns_sys::rust_main($cmain)
        }
    };
}

/// All the C implementations are gathered under this `external` module.
pub mod external {
    pub mod interface {
        pub mod cif;
        pub mod iface;
    }
}

mod interface;
mod bytestring;
mod cffi;
pub mod cjdnslog;
mod crypto;
mod rffi;
mod rtypes;
mod util;
mod gcl;
mod subnode;