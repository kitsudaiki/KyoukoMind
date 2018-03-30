#ifndef DEFINES_H
#define DEFINES_H

#define UNITTEST(IS_VAL, SHOULD_VAL) if(IS_VAL != SHOULD_VAL) {  std::cout << "Value " << #IS_VAL << " must be " << SHOULD_VAL <<" but is " << IS_VAL << '\n'; return;}

#endif // DEFINES_H
