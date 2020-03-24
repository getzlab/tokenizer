#include "get_all_calls.hpp"

using namespace std;

namespace GAC {

void gac_walker::walk_apply(const SeqLib::BamRecord& record) {
   // advance to next read if read ends before current position
   // XXX: this might become a generic function in walker
   if(record.ChrID() < curchr) continue;
   if(cur_read.ChrID() == curchr &&
      cur_read.PositionEnd() < curpos) continue;

   // advance position(s) if read starts after current position
   while((cur_read.Position() > curpos &&
	 cur_read.ChrID() == curchr) ||
	 cur_read.ChrID() > curchr) {

      // flush to disk
      
      uint32_t& alt_count = pos_cache.at(curpos).alt_count;
      uint32_t& ref_count = pos_cache.at(curpos).ref_count;
      
      float alt_frac = ((float) alt_count)/((float) alt_count + (float) ref_count) > 0.001

      uint8_t tok_i = 0;
      if(alt_count + ref_count >= 8) tok_i++; // +
      if(alt_count >= 1 && alt_frac >= 0.001) tok_i++; // 1
      if(alt_count >= 2 && alt_frac >= 0.003) tok_i++; // 2
      if(alt_count >= 3 && alt_frac >= 0.01) tok_i++; // 3
      if(alt_count >= 3 && alt_frac >= 0.03) tok_i++; // 4
      if(alt_count >= 3 && alt_frac >= 0.20) tok_i++; // 5
      if(alt_count >= 10 && alt_frac >= 0.20) tok_i++; // 5

      //"-+123456"[tok_i]
      fwrite("-+123456" + tok_i, 1, 1, outfile);

/*
- : total counts < 8 (i.e. insufficient coverage)
+ : total counts >= 8, notwithstanding higher token criteria
1 : alt count >= 1 and alt fraction >= 0.1%
2 : alt count >= 2 and alt fraction >= 0.3%
3 : alt count >= 3 and alt fraction >= 1%
4 : alt count >= 3 and alt fraction >= 3%
5 : alt count >= 3 and alt fraction >= 20%
6 : alt count >= 10 and alt fraction >= 20%
*/

      pos_cache.erase(curpos);

      // next position
      curpos++; // XXX: need to make sure this doesn't run off the end of the chromosome
   }

   // if read overlaps the current position, increment alt/refcounts along its span of the buffer
   if(cur_read.ChrID() == curchr &&
      cur_read.Position() <= curpos && 
      cur_read.PositionEnd() >= curpos) {

      // increment refcount along the whole span of the read in the buffer
      while((pos = curpos) < cur_read.PositionEnd()) pos_cache.increment(pos++, false);

      // if this read contains nonreference bases, we need to:
      // 1. increment altcount at the relevant location
      // 2. decrement refcount at that location
      if(!EDz(cur_read)) {
	 uint32_t o = 0;
	 uint32_t pos;

	 vector<uint64_t> nrp = nonref_pos(cur_read);

	 // TODO: what does the original tokenizer do with deletions? does it span the whole deletion?
	 //       if so, we need to update this.
	 for(auto& p : nrp) {
	    poscache.increment(p & 0xFFFFFFFF, true);
	    poscache.decrement(p & 0xFFFFFFFF, false);
	 }
      }
   }
}

}
