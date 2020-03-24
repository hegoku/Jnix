ENTRY(main)
SECTIONS {
	. = 0xC0100000;

	.text : {
        * (.text)
    }

    .data : {
        * (.data)
    }

    .init.text : {
        * (.init.text)
    }

    .init.data : {
        __initcall_start = .;
        *(.initcallearly.init)      
        __initcall0_start = .;          
        *(.initcall0.init)              
        *(.initcall0s.init)            
        __initcall1_start = .;          
        *(.initcall1.init)              
        *(.initcall1s.init)             
        __initcall2_start = .;          
        *(.initcall2.init)             
        *(.initcall2s.init)              
        __initcall3_start = .;          
        *(.initcall3.init)             
        *(.initcall3s.init)             
        __initcall4_start = .;          
        *(.initcall4.init)              
        *(.initcall4s.init)             
        __initcall5_start = .;          
        *(.initcall5.init)              
        *(.initcall5s.init)             
        __initcallrootfs_start = .;     
        *(.initcallrootfs.init)         
        *(.initcallrootfss.init)        
        __initcall6_start = .;          
        *(.initcall6.init)              
        *(.initcall6s.init)             
        __initcall7_start = .;          
        *(.initcall7.init)              
        *(.initcall7s.init)             
        __initcall_end = .;
    }

    .bss : {
        * (.bss)
    }
}