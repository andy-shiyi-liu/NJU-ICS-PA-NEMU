#include "cpu/cpu.h"


inline void set_ZF(uint32_t result, size_t data_size){
    result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
    cpu.eflags.ZF = (result == 0);
}

inline void set_SF(uint32_t result, size_t data_size){
    result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
    cpu.eflags.SF = sign(result);
}

inline void set_PF(uint32_t result){
    int count = 0;
    result = result & (0xFF);
    
    while(result){
        count += result & 0x1;
        result = result >> 1;
    }
    
    cpu.eflags.PF = !(count % 2);
}

inline void set_CF_add(uint32_t result, uint32_t src, size_t data_size){
    result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    cpu.eflags.CF = result < src;
}

inline void set_OF_add(uint32_t result, uint32_t src, uint32_t dest, size_t data_size){
    switch(data_size){
        case 8:
            result = sign_ext(result & 0xFF, 8);
            src = sign_ext(src & 0xFF, 8);
            dest = sign_ext(dest & 0xFF, 8);
            break;
        case 16:
            result = sign_ext(result & 0xFFFF, 16);
            src = sign_ext(src & 0xFFFF, 16);
            dest = sign_ext(dest & 0xFFFF, 16);
            break;
        default: break;
    }
    
    
    if(sign(src) == sign(dest)){
        if(sign(src) != sign(result))
            cpu.eflags.OF = 1;
        else
            cpu.eflags.OF = 0;
    }
    else{
        cpu.eflags.OF = 0;
    }
}

uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size){
    uint32_t res = 0;
    res = dest + src;
    
    set_CF_add(res, dest, data_size);
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    set_OF_add(res, src, dest, data_size);
    
    return res & (0xFFFFFFFF >> (32 - data_size));
    
}

inline void set_CF_adc(uint32_t result, uint32_t src, uint32_t cf, size_t data_size){
    result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    if(cf)
        cpu.eflags.CF = result <= src;
    else
        cpu.eflags.CF = result < src;
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size){
    // printf("src: %08x, dest: %08x, CF: %01x, data_size: %d\n", src, dest, cpu.eflags.CF, data_size);
    uint32_t res = 0;
    res = dest + src + cpu.eflags.CF;
    // printf("res: %08x\n", res);
    
    set_CF_adc(res, dest, cpu.eflags.CF, data_size);
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    set_OF_add(res, src, dest, data_size);
    
    return res & (0xFFFFFFFF >> (32 - data_size));
}

inline void set_CF_sub(uint32_t result, uint32_t dest, size_t data_size){
    // printf("In set_CF_sub:\n");
    result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    cpu.eflags.CF = result > dest;
    // printf("    dest: %08x, result: %08x, data_size: %d\n", dest, result, data_size);
    // printf("    CF: %d\n", cpu.eflags.CF);
}

void set_OF_sub(uint32_t result, uint32_t src, uint32_t dest, size_t data_size){
    //printf("In set_OF_sub:\n");
    int invalid_complement = 0;
    switch(data_size){
        case 8:
            result = sign_ext(result & 0xFF, 8);
            src = sign_ext(src & 0xFF, 8);
            dest = sign_ext(dest & 0xFF, 8);
            if(src == 0x80)
                invalid_complement = 1;
            break;
        case 16:
            result = sign_ext(result & 0xFFFF, 16);
            src = sign_ext(src & 0xFFFF, 16);
            dest = sign_ext(dest & 0xFFFF, 16);
            if(src == 0x8000)
                invalid_complement = 1;
            break;
        default: 
            if(src == 0x80000000)
                invalid_complement = 1;
            break;
    }
    //printf("    dest: %08x, result: %08x, data_size: %d\n", dest, result, data_size);
    
    if(invalid_complement){
        if(!sign(dest)){//dest >= 0, certainly overflow
            cpu.eflags.OF = 1;
            //printf("    OF: 1, invalid complement.\n");
            return;
        }
        else{
            cpu.eflags.OF = 0;
            //printf("    OF: 0, invalid complement.\n");
            return;
        }
    }
    
    src = ~src + 1;
    if(sign(src) == sign(dest)){
        if(sign(dest) != sign(result)){
            cpu.eflags.OF = 1;
            //printf("    OF: 1\n");
        }
        else{
            cpu.eflags.OF = 0;
            //printf("    OF: 0\n");
        }
    }
    else{
        cpu.eflags.OF = 0;
        //printf("    OF: 0\n");
    }
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size){
    //printf("dest: %08x, src: %08x, data_size: %d\n", dest, src, data_size);
    uint32_t res = 0;
    res = dest - src;
    //printf("res: %08x\n", res);
    
    set_CF_sub(res, dest, data_size);
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    set_OF_sub(res, src, dest, data_size);
    
    return res & (0xFFFFFFFF >> (32 - data_size));
}

inline void set_CF_sbb(uint32_t result, uint32_t dest, uint32_t cf, size_t data_size){
    // printf("In set_CF_sub:\n");
    result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    if(cf)
        cpu.eflags.CF = (result >= dest);
    else
        cpu.eflags.CF = (result > dest);
    // printf("    dest: %08x, result: %08x, data_size: %d\n", dest, result, data_size);
    // printf("    CF: %d\n", cpu.eflags.CF);
}

void set_OF_sbb(uint32_t result, uint32_t src, uint32_t dest, uint32_t cf, size_t data_size){
    //printf("In set_OF_sub:\n");
    int invalid_complement = 0;
    switch(data_size){
        case 8:
            result = sign_ext(result & 0xFF, 8);
            src = sign_ext(src & 0xFF, 8);
            dest = sign_ext(dest & 0xFF, 8);
            if(src == 0x80)
                invalid_complement = 1;
            break;
        case 16:
            result = sign_ext(result & 0xFFFF, 16);
            src = sign_ext(src & 0xFFFF, 16);
            dest = sign_ext(dest & 0xFFFF, 16);
            if(src == 0x8000)
                invalid_complement = 1;
            break;
        default: 
            if(src == 0x80000000)
                invalid_complement = 1;
            break;
    }
    //printf("    dest: %08x, result: %08x, data_size: %d\n", dest, result, data_size);
    
    if(invalid_complement){
        if(!cf){
            if(!sign(dest)){//dest >= 0, certainly overflow
                cpu.eflags.OF = 1;
                //printf("    OF: 1, invalid complement, cf = 0\n");
                return;
            }
            else{
                cpu.eflags.OF = 0;
                //printf("    OF: 0, invalid complement, cf = 0\n");
                return;
            }
        }
        else{
            if(!sign(dest) && (dest != 0)){//dest >= 0, certainly overflow
                cpu.eflags.OF = 1;
                //printf("    OF: 1, invalid complement, cf = 1\n");
                return;
            }
            else{
                cpu.eflags.OF = 0;
                //printf("    OF: 0, invalid complement, cf = 1\n");
                return;
            }
        }
    }
    
    src = ~src + 1 - cf;
    //printf("    dest: %08x, result: %08x, data_size: %d\n", dest, result, data_size);
    if(sign(src) == sign(dest)){
        if(sign(dest) != sign(result)){
            cpu.eflags.OF = 1;
            //printf("    OF: 1\n");
        }
        else{
            cpu.eflags.OF = 0;
            //printf("    OF: 0\n");
        }
    }
    else{
        cpu.eflags.OF = 0;
        //printf("    OF: 0\n");
    }
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size){
    //printf("\ndest: %08x, src: %08x, CF: %d, data_size: %d\n", dest, src, cpu.eflags.CF, data_size);
    uint32_t res = 0;
    res = dest - src - cpu.eflags.CF;
    //printf("res: %08x\n", res);
    
    set_OF_sbb(res, src, dest, cpu.eflags.CF, data_size);
    set_CF_sbb(res, dest, cpu.eflags.CF, data_size);
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    
    return res & (0xFFFFFFFF >> (32 - data_size));
}

// 返回两个操作数无符号乘法的乘积， data_size 为操作数长度（比特数），在设置标志位时有用
uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size){
    // printf("dest: %x, src: %x, data_size: %d\n", dest, src, data_size);
    uint64_t res = 0;
    src = unsign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = unsign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    res = (uint64_t)src * (uint64_t)dest;
    // printf("res: %llx\n", res);
    
    cpu.eflags.OF = cpu.eflags.CF = !((res >> data_size) == 0);
    // printf("(res >> data_size): %llx\n", (res >> data_size));
    // printf("OF: %d, CF: %d\n", cpu.eflags.OF, cpu.eflags.CF);
    
    return res;
}

// 返回两个操作数带符号乘法的乘积
int64_t alu_imul(int32_t src, int32_t dest, size_t data_size){
    // printf("dest: %x, src: %x, data_size: %d\n", dest, src, data_size);
    int64_t res = 0;
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    res = (int64_t)src * (int64_t)dest;
    // printf("res: %llx\n", res);
    
    cpu.eflags.OF = cpu.eflags.CF = (res == sign_ext(res & (0xFFFFFFFF >> (32 - data_size)), data_size));
    // printf("(res >> data_size): %llx\n", (res >> data_size));
    // printf("OF: %d, CF: %d\n", cpu.eflags.OF, cpu.eflags.CF);
    
    return res;
}

// need to implement alu_mod before testing
// 返回无符号除法 dest / src 的商，遇到 src 为0直接报错（对应Linux是Floating Point Exception）退出程序
uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size){
    //printf("dest: %llx, src: %llx, data_size: %d\n", dest, src, data_size);
    if(src == 0){
        printf("Error: Divided by 0\n");
        assert(src != 0);
    }
    
    uint32_t res = 0;
    src = unsign_ext_64(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = unsign_ext_64(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    res = dest / src;
    //printf("res: %x\n", res);
    //getchar();
    
    return res;
}

// need to implement alu_imod before testing
// 返回带符号除法 dest / src 的商，遇到 src 为0直接报错退出程序
int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size){
    //printf("dest: %llx, src: %llx, data_size: %d\n", dest, src, data_size);
    if(src == 0){
        printf("Error: Divided by 0\n");
        assert(src != 0);
    }
    
    int32_t res = 0;
    src = sign_ext_64(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = sign_ext_64(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    res = dest / src;
    //printf("res: %x\n", res);
    //getchar();
    
    return res;
}

// 返回无符号模运算 dest % src 的结果
// 实际上，整数除法和取余运算是由div或idiv指令同时完成的，我们这里为了方便，把模运算单独独立了出来
uint32_t alu_mod(uint64_t src, uint64_t dest){
    if(src == 0){
        printf("Error: Divided by 0\n");
        assert(src != 0);
    }
    
    uint32_t res = 0;
    
    res = dest % src;
    
    return res;
}

// 返回带符号模运算 dest % src 的结果
int32_t alu_imod(int64_t src, int64_t dest){
    if(src == 0){
        printf("Error: Divided by 0\n");
        assert(src != 0);
    }
    
    int32_t res = 0;
    
    res = dest % src;
    
    return res;
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size){
    uint32_t res = 0;
    res = src & dest;
    
    cpu.eflags.OF = 0;
    cpu.eflags.CF = 0;
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    
    return res & (0xFFFFFFFF >> (32 - data_size));
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size){
    uint32_t res = 0;
    res = src ^ dest;
    
    cpu.eflags.OF = 0;
    cpu.eflags.CF = 0;
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    
    return res & (0xFFFFFFFF >> (32 - data_size));
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size){
    uint32_t res = 0;
    res = src | dest;
    
    cpu.eflags.OF = 0;
    cpu.eflags.CF = 0;
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    
    return res & (0xFFFFFFFF >> (32 - data_size));
}

// 返回将 dest 算术左移 src 位后的结果， data_size 用于指明操作数长度（比特数），
// 可以是 8、16、32 中的一个用于判断标志位的取值，标志位设置参照手册说明
uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size){
    uint32_t temp = src;
    uint32_t res = 0;
    res = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    while(temp!=0){
        cpu.eflags.CF = sign(res);
        res = res << 1;
        res = sign_ext(res & (0xFFFFFFFF >> (32 - data_size)), data_size);
        temp--;
    }
    
    if(src == 1){
        cpu.eflags.OF = (cpu.eflags.CF != sign(res));
    }
    
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    
    return res & (0xFFFFFFFF >> (32 - data_size));
}

// 返回将 dest 逻辑右移 src 位后的结果（高位补零）， data_size 用于指明操作数长度，标志位设置参照手册说明
uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size){
    // printf("dest: %x, count: %d, data_size: %d\n", dest, src, data_size);
    uint32_t temp = src;
    uint32_t res = 0;
    res = unsign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    if(src == 1){
        cpu.eflags.OF = sign(res);
    }
    
    while(temp!=0){
        cpu.eflags.CF = res & 0x1;
        res = ((unsigned int)res) >> 1;
        res = sign_ext(res & (0xFFFFFFFF >> (32 - data_size)), data_size);
        temp--;
    }
    
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    
    return res & (0xFFFFFFFF >> (32 - data_size));
}

// 返回将 dest 算术右移 src 位后的结果（高位补符）， data_size 用于指明操作数长度，标志位设置参照手册说明
uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size){
    // printf("dest: %x, count: %d, data_size: %d\n", dest, src, data_size);
    uint32_t temp = src;
    uint32_t res = 0;
    res = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    while(temp!=0){
        cpu.eflags.CF = res & 0x1;
        // printf("res: %x\n", res);
        res = ((int32_t)res) >> 1;
        // printf("shifted res: %x\n", res);
        res = sign_ext(res & (0xFFFFFFFF >> (32 - data_size)), data_size);
        temp--;
    }
    
    if(src == 1){
        cpu.eflags.OF = (cpu.eflags.CF != sign(res));
    }
    
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    
    // getchar();
    return res & (0xFFFFFFFF >> (32 - data_size));
}

// 同 alu_shl() 
uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size){
    uint32_t temp = src;
    uint32_t res = 0;
    res = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    while(temp!=0){
        cpu.eflags.CF = sign(res);
        res = res << 1;
        res = sign_ext(res & (0xFFFFFFFF >> (32 - data_size)), data_size);
        temp--;
    }
    
    if(src == 1){
        cpu.eflags.OF = (cpu.eflags.CF != sign(res));
    }
    
    set_PF(res);
    set_ZF(res, data_size);
    set_SF(res, data_size);
    
    return res & (0xFFFFFFFF >> (32 - data_size));
}
