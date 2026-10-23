#
char copr[] "copyright (c) 1975 Thomas S. Duff";
#define NROWS 63
#define COLDIM 64	/* powers of 2 cause the C compiler to generate
			 * considerably better code for array subscriping.
			 */
#define NCOLS 63
#define EMPTY 0
#define BLOCKED 1
#define USED 2
char maze[NROWS][COLDIM];
int nrows;
int ncols;
#define NDIRS 4
#define NPERMS 24 /* the number of permutations of 4 objects */
int perm[NPERMS][NDIRS]{
	1,2,3,0,
	3,2,1,0,
	2,3,1,0,
	1,3,2,0,
	3,1,2,0,
	2,1,3,0,
	1,2,0,3,
	3,2,0,1,
	2,3,0,1,
	1,3,0,2,
	3,1,0,2,
	2,1,0,3,
	1,0,2,3,
	3,0,2,1,
	2,0,3,1,
	1,0,3,2,
	3,0,1,2,
	2,0,1,3,
	0,1,2,3,
	0,3,2,1,
	0,2,3,1,
	0,1,3,2,
	0,3,1,2,
	0,2,1,3};
struct{
	int x;
	int y;
}dir[NDIRS]{
	0,1,
	1,0,
	0,-1,
	-1,0};
int tvec[2];
int maxlength 0;
int maxx, maxy;
main(argc, argv)
char *argv[];
{
/*
 * Seed the random number generator.
 * Initialize the maze array so that it is all empty
 * except for the edges, which are blocked to
 * prevent escape.
 * Mark the entrance cell used and start building the maze.
 * Finally, open the exit and leave.
 */
	register i, j;
	if(argc<2)
		nrows=NROWS;
	else{
		nrows=atoi(argv[1])*2+1;
		if(nrows<3||nrows>NROWS)
			nrows=NROWS;
	}
	if(argc<3)
		ncols=NCOLS;
	else{
		ncols=atoi(argv[2])*2+1;
		if(ncols<3||ncols>NCOLS)
			ncols=NCOLS;
	}
	time(tvec);
	srand(tvec[0]^tvec[1]);
	for(i=1;i<nrows;i++){
		for(j=1;j<ncols-1;j++)
			if((i&1)|(j&1))
				maze[i][j]=EMPTY;
			else
				maze[i][j]=BLOCKED;
		maze[i][0]=BLOCKED;
		maze[i][ncols-1]=BLOCKED;
	}
	for(j=0;j<ncols;j++){
		maze[0][j]=BLOCKED;
		maze[nrows-1][j]=BLOCKED;
	}
	maze[0][1]=USED;
	gener(1,1,-1);
	if(maxx==1)
		maxx=0;
	else if(maxx==nrows-2)
		maxx++;
	else if(maxy==1)
		maxy=0;
	else if(maxy==ncols-2)
		maxy++;
	maze[maxx][maxy]=USED;
	prmaze();
}
gener(x,y,length)
/*
 * If (x,y) is on the edge and the path is longer than any other
 * ending on the edge, make note of the fact.
 * Generate a permutation of four directions.
 * If a direction is blocked, ignore it.
 * If any of its neighbours (other than here)
 * is used, block it.
 * Otherwise, move there and repeat the procedure recursively.
 */
{
	register n, newx, newy;
	int i, j, tx, ty;
	if((x==1 || x==nrows-2 || y==1 || y==ncols-2) && length>maxlength){
		maxx=x;
		maxy=y;
		maxlength=length;
	}
	maze[x][y]=USED;
	n=rand()%NPERMS;
	for(i=0;i<NDIRS;i++){
		newx=x+dir[perm[n][i]].x;
		newy=y+dir[perm[n][i]].y;
		if(maze[newx][newy]==EMPTY){
			for(j=0;j<NDIRS;j++){
				tx=newx+dir[j].x;
				ty=newy+dir[j].y;
				if((tx!=x || ty!=y)
				&& maze[tx][ty]==USED){
					maze[newx][newy]=BLOCKED;
					goto cont_dirloop;
				}
			}
			maze[newx][newy]=USED;
			gener(newx,newy,length+1);
		}
cont_dirloop:	;
	}
}
prmaze(){
	register i,j;
	for(i=0;i<nrows;i++){
		for(j=0;j<ncols;j++)
			if(maze[i][j]==USED)
				putchar(' ');
			else if((i&1)==0 && (j&1)==0)
				putchar(':');
			else if((i&1)==0)
				putchar('-');
			else
				putchar('I');
		putchar('\n');
	}
}
