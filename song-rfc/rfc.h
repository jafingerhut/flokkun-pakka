#define MAXRULES 10000
#define MAXDIMENSIONS 5
#define MAXCHUNKS 7
#define MAXTABLE 5000000
#define FILTERSIZE 18

struct eq {
  unsigned int numrules;
  int *rulelist;
};

struct range{
  unsigned low;
  unsigned high;
};

struct pc_rule{
  struct range field[MAXDIMENSIONS];
};
