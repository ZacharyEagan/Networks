#include "smartalloc.h"
#include <vector>
#include <iostream>

static std::vector<int> example;
static std::vector<int, STLsmartalloc<int> > example2;
static std::vector<int, STLsmartalloc<int> > *example3;

int main()
{
   // STL containers without the smartalloc allocator class.  Lots of leaks.
   example.push_back(0);
   example.push_back(1);
   example.push_back(2);
   example.push_back(3);
   std::cout << "Space before global 'example' is cleared: " << report_space() << std::endl;
   example.clear();
   std::cout << "Space after global 'example' is cleared: " << report_space() << std::endl;

   // STL containers with the smartalloc allocator class.  Fewer leaks because all nodes
   // are accounted for correctly.  vector meta-data is still leaking, however, because the vector itself was
   // never destructed.
   example2.push_back(0);
   example2.push_back(1);
   example2.push_back(2);
   example2.push_back(3);
   std::cout << "Space before global 'example2' is cleared: " << report_space() << std::endl;
   example2.clear();
   std::cout << "Space after global 'example2' is cleared: " << report_space() << std::endl;

   // STL containers with the smartalloc allocator class.  No leaks.  Deleting the global pointer
   // forces the destruction of the vector.
   example3 = new std::vector<int, STLsmartalloc<int> >();
   example3->push_back(0);
   example3->push_back(1);
   example3->push_back(2);
   example3->push_back(3);
   std::cout << "Space before global 'example3' is cleared: " << report_space() << std::endl;
   example3->clear();
   std::cout << "Space after global 'example3' is cleared: " << report_space() << std::endl;
   delete example3;
   std::cout << "Space after global 'example3' is deleted: " << report_space() << std::endl;

   return 0;
}
