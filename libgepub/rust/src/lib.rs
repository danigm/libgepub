// TODO: manage errors, remove all unwraps

extern crate glib;
extern crate libc;
extern crate epub;

use std::mem;
use epub::doc::EpubDoc;
use self::glib::translate::*;


#[no_mangle]
pub extern "C" fn epub_new(path: *const libc::c_char) -> *mut EpubDoc {
    let my_path = unsafe { &String::from_glib_none(path) };
    let doc = EpubDoc::new(my_path);
    let doc = doc.unwrap();
    Box::into_raw(Box::new(doc))
}

#[no_mangle]
pub unsafe extern "C" fn epub_destroy(raw_doc: *mut EpubDoc) {
    assert!(!raw_doc.is_null());
    let _ = Box::from_raw(raw_doc);
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_resource(doc: *mut EpubDoc,
                                           path: *const libc::c_char,
                                           size: *mut i32)
                                           -> *mut u8 {
    assert!(!doc.is_null());
    let my_path = &String::from_glib_none(path);

    let mut v = (*doc).get_resource_by_path(my_path).unwrap();

    *size = v.len() as i32;

    v.shrink_to_fit();
    let ptr = v.as_mut_ptr();
    mem::forget(v);

    ptr
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_resource_by_id(doc: *mut EpubDoc,
                                                 id: *const libc::c_char,
                                                 size: *mut i32)
                                                 -> *mut u8 {
    assert!(!doc.is_null());
    let my_id = &String::from_glib_none(id);

    let mut v = (*doc).get_resource(my_id).unwrap();
    *size = v.len() as i32;

    v.shrink_to_fit();
    let ptr = v.as_mut_ptr();
    mem::forget(v);

    ptr
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_metadata(doc: *mut EpubDoc,
                                           mdata: *const libc::c_char)
                                           -> *const libc::c_char {
    assert!(!doc.is_null());
    let mut ret = String::from("");
    let my_mdata = &String::from_glib_none(mdata);
    if let Some(mdata) = (*doc).metadata.get(my_mdata) {
        ret = mdata.clone();
    }

    ret.to_glib_full()
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_resource_mime(doc: *mut EpubDoc,
                                                path: *const libc::c_char)
                                                -> *const libc::c_char {
    assert!(!doc.is_null());
    let mut ret = String::from("");
    let my_path = &String::from_glib_none(path);
    if let Ok(m) = (*doc).get_resource_mime_by_path(my_path) {
        ret = m.clone();
    }
    ret.to_glib_full()
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_resource_mime_by_id(doc: *mut EpubDoc,
                                                      id: *const libc::c_char)
                                                      -> *const libc::c_char {
    assert!(!doc.is_null());
    let mut ret = String::from("");
    let my_id = &String::from_glib_none(id);
    if let Ok(m) = (*doc).get_resource_mime(my_id) {
        ret = m.clone();
    }
    ret.to_glib_full()
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_current_mime(doc: *mut EpubDoc) -> *const libc::c_char {
    assert!(!doc.is_null());
    let mut ret = String::from("");
    if let Ok(m) = (*doc).get_current_mime() {
        ret = m.clone();
    }
    ret.to_glib_full()
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_current(doc: *mut EpubDoc, size: *mut i32) -> *mut u8 {
    assert!(!doc.is_null());
    let mut v = (*doc).get_current().unwrap();
    *size = v.len() as i32;

    v.shrink_to_fit();
    let ptr = v.as_mut_ptr();
    mem::forget(v);

    ptr
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_current_with_epub_uris(doc: *mut EpubDoc,
                                                         size: *mut i32)
                                                         -> *mut u8 {
    assert!(!doc.is_null());
    let mut v = (*doc).get_current_with_epub_uris().unwrap();
    *size = v.len() as i32;

    v.shrink_to_fit();
    let ptr = v.as_mut_ptr();
    mem::forget(v);

    ptr
}

#[no_mangle]
pub unsafe extern "C" fn epub_set_page(doc: *mut EpubDoc, n: usize) {
    (*doc).set_current_page(n);
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_num_pages(doc: *mut EpubDoc) -> usize {
    (*doc).get_num_pages()
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_page(doc: *mut EpubDoc) -> usize {
    (*doc).get_current_page()
}

#[no_mangle]
pub unsafe extern "C" fn epub_next_page(doc: *mut EpubDoc) -> bool {
    match (*doc).go_next() {
        Ok(_) => return true,
        Err(_) => return false,
    }
}

#[no_mangle]
pub unsafe extern "C" fn epub_prev_page(doc: *mut EpubDoc) -> bool {
    match (*doc).go_prev() {
        Ok(_) => return true,
        Err(_) => return false,
    }
}

#[no_mangle]
pub unsafe extern "C" fn epub_get_cover(doc: *mut EpubDoc) -> *const libc::c_char {
    let mut ret = String::from("");
    if let Ok(cid) = (*doc).get_cover_id() {
        if let Some(m) = (*doc).resources.get(&cid) {
            ret = m.0.clone();
        }
    }

    ret.to_glib_full()
}

#[no_mangle]
pub unsafe extern "C" fn epub_resource_path(doc: *mut EpubDoc,
                                            id: *const libc::c_char)
                                            -> *const libc::c_char {
    let my_id = &String::from_glib_none(id);
    let mut ret = String::from("");
    if let Some(m) = (*doc).resources.get(my_id) {
        ret = m.0.clone();
    }
    ret.to_glib_full()
}

#[no_mangle]
pub unsafe extern "C" fn epub_current_path(doc: *mut EpubDoc) -> *const libc::c_char {
    let mut ret = String::from("");
    if let Ok(m) = (*doc).get_current_path() {
        ret = m.clone();
    }
    ret.to_glib_full()
}

#[no_mangle]
pub unsafe extern "C" fn epub_current_id(doc: *mut EpubDoc) -> *const libc::c_char {
    let mut ret = String::from("");
    if let Ok(m) = (*doc).get_current_id() {
        ret = m.clone();
    }
    ret.to_glib_full()
}
