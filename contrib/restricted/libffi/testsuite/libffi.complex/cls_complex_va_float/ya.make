# Generated by devtools/yamaker. 
 
PROGRAM() 
 
WITHOUT_LICENSE_TEXTS()
 
OWNER(
    borman
    g:cpp-contrib
)

LICENSE(GPL-2.0-only)

PEERDIR( 
    contrib/restricted/libffi 
) 
 
NO_COMPILER_WARNINGS() 
 
NO_RUNTIME() 
 
SRCDIR(contrib/restricted/libffi/testsuite/libffi.complex) 
 
SRCS( 
    cls_complex_va_float.c 
) 
 
END() 
