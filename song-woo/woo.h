#define MAXNODES 4000000
#define MAXRULES 20000
#define MAXBUCKETS 40 
#define MAXDIMENSIONS 5
#define RULESIZE 18 
#define NODESIZE 4
#define RULEPTSIZE 2 

struct range{
  unsigned low;
  unsigned high;
};

struct pc_rule{
  int    id;
  int    priority;
  struct range field[MAXDIMENSIONS];
};

struct prefix {
  unsigned int value[16];
  unsigned int length[16];
  unsigned int nvalid;
};

