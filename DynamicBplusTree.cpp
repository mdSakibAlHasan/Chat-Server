#include<bits/stdc++.h>
using namespace std;
using namespace std::chrono;

int n,mx; //number of child;
struct mystruct
{
    int currentNode;
    mystruct *parent,*next;
    string *english,*bangla;
    bool lf;
    mystruct **point;
} ;
mystruct *root,*firstLeaf;


mystruct* createNode()          //increase adress size
{
    mystruct *m = new mystruct();

    m->point = new mystruct *[n+1];
    m->english=new string[n];
    m->bangla=new string[n];

    m->currentNode = 0;
    m->parent = NULL;
    m->next = NULL;
    m->lf = true;
    return m;
}
mystruct *findLeaf(mystruct *tempRt,string english, int *counter)
{
    int sum=0;
    while(tempRt->lf==false)
    {
        int i;
        for(i=0; i<tempRt->currentNode; i++) 
            if(english<tempRt->english[i]) break;

        tempRt = tempRt->point[i];
        sum++;
    }
    *counter = sum;
    return tempRt;

}

void insertValueAndPoint(mystruct *parent,string value,mystruct *right)
{                                       //add with parents node
    int i=parent->currentNode-1;
    for(;i>=0; i--)
    {
        if(parent->english[i]<=value) break;
        else
        {
            parent->english[i+1] = parent->english[i];
            parent->point[i+2] = parent->point[i+1];
        }
    }
    parent->english[i+1] = value;
    parent->point[i+2] = right;
    parent->currentNode++;
}

void insertMiddle(mystruct *parent,string value,mystruct *left,mystruct *right)
{
    if(parent==NULL)        //create a root
    {
        parent = createNode();
        parent->english[0] = value;
        parent->point[0] = left;
        parent->point[1] = right;
        parent->currentNode++;
        parent->lf = false;
        root = parent;
        left->parent = parent;
        right->parent = parent;
        return;
    }
    right->parent = parent;
    insertValueAndPoint(parent,value,right);
    if(parent->currentNode==mx)
    {
        mystruct *splitNode = createNode();
        splitNode->lf = false;
        int j=0;
        for(int i=parent->currentNode-(n)/2;i<mx; i++)
        {
            splitNode->english[j] = parent->english[i];
            if(j==0)
            {
                splitNode->point[0] = parent->point[i];
                splitNode->point[0]->parent = splitNode;
            }
            splitNode->point[j+1] = parent->point[i+1];
            splitNode->point[j+1]->parent = splitNode;
            j++;
        }
        parent->currentNode-=(n)/2+1;
        splitNode->currentNode = (n)/2;
        insertMiddle(parent->parent,parent->english[parent->currentNode],parent,splitNode);
    }

}

void insertLeaf(string english,string bangla)
{
    int counter=0;
    mystruct *leaf = findLeaf(root,english,&counter);        //find leaf where it can be fit
    int i= leaf->currentNode-1;
    if(i>-1) {
    for(; i>=0; i--)
    {
    	if(english<leaf->english[i])                //fit in leaf
    	{
    		leaf->english[i+1] = leaf->english[i];
    		leaf->bangla[i+1] = leaf->bangla[i];
    	}
    	else break;
    }
    }
    leaf->english[i+1] = english;
    leaf->bangla[i+1] = bangla;
    leaf->currentNode++;

    if(leaf->currentNode==mx)               //split and create two leaf
    {
        mystruct *splitNode = createNode();
        int j=0;
        for(int i=leaf->currentNode-n/2;i<mx; i++)
        {
            splitNode->english[j] = leaf->english[i];
            splitNode->bangla[j] = leaf->bangla[i];
            j++;
        }
        leaf->currentNode-=n/2;
        splitNode->currentNode = n/2;
        splitNode->next = leaf->next;
        leaf->next = splitNode;
        insertMiddle(leaf->parent,splitNode->english[0],leaf,splitNode);
    }
}

int main(void)
{
	cout << "number of Child:" << endl;
	cin >> n;
	mx=n;
    //cout<<n<<" "<<mx<<endl;
    root = createNode();
    mystruct *leaf;
    int i=0,counter;
    string english,bangla,searchEnglish;
    ifstream ifile;
    ifile.open("dictionary.txt");
    if(!ifile) return 0;
    while(ifile>>english)
    {
    	getline(ifile,bangla);
    	insertLeaf(english,bangla);
    }


    cout<<"enter text to search: ";
    while(cin>>searchEnglish)
    {
        auto start = high_resolution_clock::now();

    	 leaf= findLeaf(root,searchEnglish,&counter);
    	 for(i=0; i<leaf->currentNode; i++) if(searchEnglish==leaf->english[i]) break;
    	 if(i==leaf->currentNode) cout<<"no word founds\n";
    	 else cout<<searchEnglish<<" "<<leaf->bangla[i]<<endl;

         auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
	    cout << "Time taken for searching "<< duration.count()<< " microseconds and steps "<<counter<<endl;
    	 cout<<"enter text to search: ";
    }


}
