0:			t0 = -2.3;
1:			t1 = -9.1;
2:			t2 = 40;
3:			param t2;
4:			call memalloc 1;
5:			pop_param t3;
6:			*t3 = t0;
7:			t3 = t3 + 8;
8:			*t3 = 3.14;
9:			t3 = t3 + 8;
10:			*t3 = 0.9;
11:			t3 = t3 + 8;
12:			*t3 = 11;
13:			t3 = t3 + 8;
14:			*t3 = t1;
15:			t3 = t3 + 8;
16:			t3 = t3 - 40;
17:			data = t3;
18:	global.compute_min:
19:		beginfunc;
20:			stack_pointer = stack_pointer - 20;
21:			min_value = None;
22:			i = 0;
23:			param 10;
24:			param 0;
25:			stack_pointer = stack_pointer - 16;
26:			call global.range 2;
27:			stack_pointer = stack_pointer + 16;
28:			pop_param t4;
29:			t7 = 0;
30:	L2:
31:			if t7 LT 10 goto L5;
32:			goto L1;
33:	L5:
34:			if i LT i goto L4;
35:			goto L3;
36:	L4:
37:			t6 = i * 8;
38:			t6 = data + t6;
39:			t5 = *t6;
40:			t5 = min_value;
41:			goto L3;
42:	L3:
43:			t7 = t7 + 1;
44:			goto L2;
45:	L1:
46:		endfunc;
