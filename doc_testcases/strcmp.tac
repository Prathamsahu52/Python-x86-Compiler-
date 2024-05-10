0:		.data;
1:	.str0:
2:		.string "One or both strings are empty.";
3:	.str1:
4:		.string "The strings are equal.";
5:	.str10:
6:		.string "__main__";
7:	.str2:
8:		.string "The first string is lexicographically smaller than the second string.";
9:	.str3:
10:		.string "The first string is lexicographically greater than the second string.";
11:	.str4:
12:		.string "apple";
13:	.str5:
14:		.string "banana";
15:	.str6:
16:		.string "world";
17:	.str7:
18:		.string "hello";
19:	.str8:
20:		.string "python";
21:	.str9:
22:		.string "python";
23:	global.string_comparison:
24:		beginfunc;
25:			pop_param string1;
26:			pop_param string2;
27:			pop_param n1;
28:			pop_param n2;
29:			if n1 EQ 0 goto L2;
30:			goto L8;
31:	L8:
32:			if n2 EQ 0 goto L2;
33:			goto L5;
34:	L2:
35:			t0 = ;
36:		leave;
37:			RETURN t0;
38:		return;
39:			goto L1;
40:	L5:
41:			if string1 EQ string2 goto L3;
42:			goto L7;
43:	L3:
44:			t2 = ;
45:		leave;
46:			RETURN t2;
47:		return;
48:			goto L1;
49:	L7:
50:			if string1 LT string2 goto L4;
51:			goto L6;
52:	L4:
53:			t1 = ;
54:		leave;
55:			RETURN t1;
56:		return;
57:			goto L1;
58:	L6:
59:			t3 = ;
60:		leave;
61:			RETURN t3;
62:		return;
63:	L1:
64:		endfunc;
65:	global.main:
66:		beginfunc;
67:			t4 = ;
68:			s1 = t4;
69:			t5 = ;
70:			s2 = t5;
71:			param 6;
72:			param 5;
73:			param s2;
74:			param s1;
75:			call global.string_comparison 4;
76:			ret t7;
77:			result = t7;
78:		print_string;
79:			param result;
80:			call global.print 1;
81:			ret ;
82:			t9 = ;
83:			s3 = t9;
84:			t10 = ;
85:			s4 = t10;
86:			param 5;
87:			param 6;
88:			param s4;
89:			param s3;
90:			call global.string_comparison 4;
91:			ret t12;
92:			result = t12;
93:		print_string;
94:			param result;
95:			call global.print 1;
96:			ret ;
97:			t14 = ;
98:			s5 = t14;
99:			t15 = ;
100:			s6 = t15;
101:			param 6;
102:			param 6;
103:			param s6;
104:			param s5;
105:			call global.string_comparison 4;
106:			ret t17;
107:			result = t17;
108:		print_string;
109:			param result;
110:			call global.print 1;
111:			ret ;
112:		endfunc;
113:			t19 = ;
114:			if __name__ EQ t19 goto L10;
115:			goto L9;
116:	L10:
117:			call global.main 0;
118:			ret ;
119:			goto L9;
120:	L9:
