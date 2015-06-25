
int test (int h, int i, double j){
	j = h + i;
}

int b;
int main () {
	int a;
	int c ;
	int d;
	
	a  =  0 ;
	b =  1 ;
	c  =  a + b + 2;
	d = test (a,c,d);
	if (a < b) {
		b = 3;
	}
	//print a;
	//print b;
	return 0 ;
}
