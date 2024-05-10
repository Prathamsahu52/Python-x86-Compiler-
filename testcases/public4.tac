0:		.data;
1:	.str0:
2:		.string "Hello";
3:	.str1:
4:		.string "Element is present at index:";
5:	.str2:
6:		.string "Element is not present";
7:	.str3:
8:		.string "__main__";
9:	global.binarySearch:
10:		beginfunc;
11:			pop_param array;
12:			pop_param x;
13:			pop_param low;
14:			pop_param high;
15:	L2:
16:			if low LE high goto L3;
17:			goto L1;
18:	L3:
19:			t0 = .str0;
20:		print_string;
21:			param t0;
22:			call global.print 1;
23:			ret ;
24:			t2 = high / 2;
25:			t3 = low + t2;
26:			mid = t3;
27:			goto L2;
28:	L1:
29:			t4 = -1;
30:		leave;
31:			RETURN t4;
32:		return;
33:		endfunc;
34:	global.main:
35:		beginfunc;
36:			param 56;
37:			call memalloc 1;
38:			ret t6;
39:			*t6 = 3;
40:			t6 = t6 + 8;
41:			*t6 = 4;
42:			t6 = t6 + 8;
43:			*t6 = 5;
44:			t6 = t6 + 8;
45:			*t6 = 6;
46:			t6 = t6 + 8;
47:			*t6 = 7;
48:			t6 = t6 + 8;
49:			*t6 = 8;
50:			t6 = t6 + 8;
51:			*t6 = 9;
52:			t6 = t6 + 8;
53:			t6 = t6 - 56;
54:			array = t6;
55:			param 1;
56:			param 0;
57:			param 4;
58:			param array;
59:			call global.binarySearch 4;
60:			ret t8;
61:			result = t8;
62:			t9 = -1;
63:			if result NE t9 goto L5;
64:			goto L6;
65:	L5:
66:			t10 = .str1;
67:		print_string;
68:			param t10;
69:			call global.print 1;
70:			ret ;
71:			param result;
72:			call global.print 1;
73:			ret ;
74:			goto L4;
75:	L6:
76:			t13 = .str2;
77:		print_string;
78:			param t13;
79:			call global.print 1;
80:			ret ;
81:	L4:
82:		endfunc;
83:			t15 = .str3;
84:			if __name__ EQ t15 goto L8;
85:			goto L7;
86:	L8:
87:			call global.main 0;
88:			ret ;
89:			goto L7;
90:	L7:
