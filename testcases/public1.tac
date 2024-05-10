0:	global.bubbleSort:
1:		beginfunc;
2:			stack_pointer = stack_pointer - 29;
3:			i = 0;
4:			a = \"abcd\";
5:			t0 = 28;
6:			param t0;
7:			call memalloc 1;
8:			pop_param t1;
9:			*t1 = 64;
10:			t1 = t1 + 4;
11:			*t1 = 34;
12:			t1 = t1 + 4;
13:			*t1 = 25;
14:			t1 = t1 + 4;
15:			*t1 = 12;
16:			t1 = t1 + 4;
17:			*t1 = 22;
18:			t1 = t1 + 4;
19:			*t1 = 11;
20:			t1 = t1 + 4;
21:			*t1 = 90;
22:			t1 = t1 + 4;
23:			t1 = t1 - 28;
24:			array = t1;
25:			param 6;
26:			stack_pointer = stack_pointer - 8;
27:			call global.range 1;
28:			stack_pointer = stack_pointer + 8;
29:			pop_param t2;
30:			t20 = 0;
31:	L2:
32:			if t20 LT 6 goto L8;
33:			goto L1;
34:	L8:
35:			swapped = False;
36:			j = 0;
37:			param 10;
38:			param 0;
39:			stack_pointer = stack_pointer - 16;
40:			call global.range 2;
41:			stack_pointer = stack_pointer + 16;
42:			pop_param t3;
43:			t19 = 0;
44:	L4:
45:			if t19 LT 10 goto L7;
46:			goto L3;
47:	L7:
48:			t5 = j * 4;
49:			t5 = array + t5;
50:			t4 = *t5;
51:			t6 = j + 1;
52:			t8 = t6 * 4;
53:			t8 = array + t8;
54:			t7 = *t8;
55:			if t4 GT t7 goto L6;
56:			goto L5;
57:	L6:
58:			t10 = j * 4;
59:			t10 = array + t10;
60:			t9 = *t10;
61:			temp = t9;
62:			t12 = j * 4;
63:			t12 = array + t12;
64:			t11 = *t12;
65:			t13 = j + 1;
66:			t15 = t13 * 4;
67:			t15 = array + t15;
68:			t14 = *t15;
69:			t11 = t14;
70:			t16 = j + 1;
71:			t18 = t16 * 4;
72:			t18 = array + t18;
73:			t17 = *t18;
74:			t17 = temp;
75:			swapped = True;
76:			goto L5;
77:	L5:
78:			goto L3;
79:			t19 = t19 + 1;
80:			goto L4;
81:	L3:
82:			t20 = t20 + 1;
83:			goto L2;
84:	L1:
85:		endfunc;
