#define MAXRULES 10000
#define MAXDIMENSIONS 5

struct range{
  unsigned low;
  unsigned high;
};

struct pc_rule{
  struct range field[MAXDIMENSIONS];
};
