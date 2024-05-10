0:		.data;
1:	.str0:
2:		.string "SLR name:";
3:	.str1:
4:		.string "CLR name:";
5:	.str2:
6:		.string "LALR name:";
7:	.str3:
8:		.string "LALR";
9:	.str4:
10:		.string "CLR";
11:	.str5:
12:		.string "Shift-Reduce";
13:	.str6:
14:		.string "__main__";
15:	ShiftReduceParser:
16:		beginclass;
17:	ShiftReduceParser.__init__:
18:		beginfunc;
19:			pop_param self;
20:			pop_param name_;
21:			t0 = self + 0;
22:			*t0 = name_;
23:		endfunc;
24:		endclass;
25:	LR0Parser:
26:		beginclass;
27:	LR0Parser.__init__:
28:		beginfunc;
29:			pop_param self;
30:			pop_param myname_;
31:			pop_param parentname_;
32:			t1 = self + 0;
33:			*t1 = myname_;
34:			param 8;
35:			call memalloc 1;
36:			ret t3;
37:			param parentname_;
38:			param self;
39:			param t3;
40:			call ShiftReduceParser.__init__ 2;
41:			ret ;
42:		endfunc;
43:		endclass;
44:	CLRParser:
45:		beginclass;
46:	CLRParser.__init__:
47:		beginfunc;
48:			pop_param self;
49:			pop_param myname_;
50:			pop_param parentname_;
51:			t4 = self + 0;
52:			*t4 = myname_;
53:			param 8;
54:			call memalloc 1;
55:			ret t6;
56:			param parentname_;
57:			param self;
58:			param t6;
59:			call ShiftReduceParser.__init__ 2;
60:			ret ;
61:		endfunc;
62:		endclass;
63:	LALRParser:
64:		beginclass;
65:	LALRParser.__init__:
66:		beginfunc;
67:			pop_param self;
68:			pop_param myname_;
69:			pop_param clrname_;
70:			pop_param srname_;
71:			t7 = self + 0;
72:			*t7 = myname_;
73:			param 16;
74:			call memalloc 1;
75:			ret t9;
76:			param srname_;
77:			param clrname_;
78:			param self;
79:			param t9;
80:			call CLRParser.__init__ 3;
81:			ret ;
82:		endfunc;
83:	LALRParser.print_name:
84:		beginfunc;
85:			pop_param self;
86:			t10 = .str0;
87:		print_string;
88:			param t10;
89:			call global.print 1;
90:			ret ;
91:			t12 = self + 16;
92:			t14 = *t12;
93:		print_string;
94:			param t14;
95:			call global.print 1;
96:			ret ;
97:			t15 = .str1;
98:		print_string;
99:			param t15;
100:			call global.print 1;
101:			ret ;
102:			t17 = self + 8;
103:			t19 = *t17;
104:		print_string;
105:			param t19;
106:			call global.print 1;
107:			ret ;
108:			t20 = .str2;
109:		print_string;
110:			param t20;
111:			call global.print 1;
112:			ret ;
113:			t22 = self + 0;
114:			t24 = *t22;
115:		print_string;
116:			param t24;
117:			call global.print 1;
118:			ret ;
119:		endfunc;
120:		endclass;
121:	global.main:
122:		beginfunc;
123:			param 24;
124:			call memalloc 1;
125:			ret t29;
126:			t27 = .str5;
127:			t26 = .str4;
128:			t25 = .str3;
129:			param t27;
130:			param t26;
131:			param t25;
132:			param t29;
133:			call LALRParser.__init__ 3;
134:			ret ;
135:			obj = t29;
136:			param obj;
137:			call LALRParser.print_name 0;
138:			ret ;
139:		endfunc;
140:			t31 = .str6;
141:			if __name__ EQ t31 goto L2;
142:			goto L1;
143:	L2:
144:			call global.main 0;
145:			ret ;
146:			goto L1;
147:	L1:
