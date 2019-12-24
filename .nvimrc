let options = '
\ -std=c++2a
\
\ -D LINT
\
\ -I include
\ -I source/grev-load
\ -I test
\
\ -I submodule/openreil/libopenreil/include
\ -I submodule/z3/src/api
\'

let g:ale_cpp_clang_options = options
let g:ale_cpp_clangtidy_options = options
let g:ale_cpp_gcc_options = options
