#define MAXRULES 10000
#define MAXDIMENSIONS 5
#define FILTERSIZE 18
struct range{
  unsigned low;
  unsigned high;
};

struct tuple{
  unsigned int tuplesize;
  int siplen;
  int diplen;
  int protolen;
  int splen;
  int dplen;
  int *rulelist;
};

struct pc_rule{
  int id;
  int priority;
  struct range field[MAXDIMENSIONS];
};

struct pc_rule_tuple{
  int ruleid;
  struct range field[MAXDIMENSIONS];
  int tupleid;
};

struct prefix {
  unsigned int value[16];
  unsigned int length[16];
  unsigned int nvalid;
};

