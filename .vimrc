set tabstop=4

function! InsertHeader()
    let filename = expand('%:t')
    let header_filename = substitute(filename, '\.c$', '.h', '')  " .c → .h
    let guard_name = '_' . toupper(substitute(filename, '\.', '_', 'g')) . '_'

    " 检查文件是否非空
    if line('$') > 1 || getline(1) != ''
        echo "File not empty. Header not inserted."
        return
    endif

    " 插入文件头注释
    call setline(1, '/*')
    call append(1, " * Copyright (C) by wuminghui")
    call append(2, " *")
    call append(3, ' * File      : ' . filename)
    call append(4, " * Version   : 1.0")
    call append(5, ' * Author    : ' . ($USER == '' ? 'your_name' : $USER))
    call append(6, ' * Created   : ' . strftime('%Y-%m-%d'))
    call append(7, " * Description : ")
    call append(8, ' */')
    call append(9, '')


    " 插入头文件保护宏
    if &filetype == 'h' || &filetype == 'hpp'
        call append(10, '#ifndef ' . guard_name)
        call append(11, '#define ' . guard_name)
        call append(12, '')

        " 如果是C/C++文件，添加extern "C"兼容声明
        if &filetype == 'c' || &filetype == 'cpp'
            call append(13, '#ifdef __cplusplus')
            call append(14, 'extern "C" {')
            call append(15, '#endif')
            call append(16, '')
        endif

        " 在文件末尾添加#endif
        call append(line('$'), '')
        call append(line('$'), '#endif /* ' . guard_name . ' */')
    endif

    if &filetype == 'c' 
        call append(10, '#include "' . header_filename . '"')
    endif

endfunction

autocmd BufNewFile *.h,*.hpp,*c call InsertHeader()