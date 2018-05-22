/* 变量 */
a = 10 + 2.2
b = a
c = b + a + 3
d = 5
str = "hello world"

/* if */
if(a < 10) {
    b = 1
}
else {
    b = 2
}

/* while */
while(a < 5 && a > 1) {
    a = a + 1
    b = a
    c = b + a
}

/* 嵌套if */
if(a < 20) {
    if(a > 10) {
        a = 5
    }
    else {
        a = 10
    }
}
else {
    a = 20
}

/* 嵌套while */
while(a < 5) {
    while(b > 1) {
        b = b + 1
    }
}

