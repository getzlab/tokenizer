#include "walker/poscache.hpp"
#include "walker/walker.hpp"

namespace GAC {

typedef struct p {
   uint32_t alt_count;
   uint32_t ref_count;
} pileup_t;

class altrefcount : public walker::pos_cache<pileup_t> {
   public:
   void increment(uint64_t pos, bool is_alt) {
      if(!this->contains(pos)) {
	 this->insert(pos, (pileup_t) { (uint32_t) is_alt, (uint32_t) !is_alt });
      } else {
	 if(is_alt) (this->at(pos).alt_count)++;
	 else (this->at(pos).ref_count)++;
      }
   }

   void decrement(uint64_t pos, bool is_alt) {
      assert(this->contains(pos));
      if(is_alt) (this->at(pos).alt_count)--;
      else (this->at(pos).ref_count)--;
   }
};

class gac_walker : public walker::walker {
   public:
   // we may not need to define our own constructor here
   // gac_walker(const string& bam_in, const string& ref_fa) : walker(bam_in, ref_fa) {}
   void walk_apply(const SeqLib::BamRecord& record);

   protected:
   altrefcount counter;
   uint32_t curpos = 0;
   uint16_t curchr = 0;
}

}
