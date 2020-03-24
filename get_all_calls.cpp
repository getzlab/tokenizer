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
      float alt_frac;
      uint32_t alt_count;
      uint32_t ref_count;

      if(pos_cache.contains(curpos)) {
	 alt_count = pos_cache.at(curpos).alt_count;
	 ref_count = pos_cache.at(curpos).ref_count;
	 alt_frac = ((float) alt_count)/((float) alt_count + (float) ref_count);
      } else {
	 alt_count = 0;
	 ref_count = 0;
	 alt_frac = 0.0;
      } 

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

      uint8_t tok_i = 0;
      if(alt_count + ref_count >= 8) { tok_i++; // +
      if(alt_count >= 1 && alt_frac >= 0.001) { tok_i++; // 1
      if(alt_count >= 2 && alt_frac >= 0.003) { tok_i++; // 2
      if(alt_count >= 3 && alt_frac >= 0.01) { tok_i++; // 3
      if(alt_count >= 3 && alt_frac >= 0.03) { tok_i++; // 4
      if(alt_count >= 3 && alt_frac >= 0.20) { tok_i++; // 5
      if(alt_count >= 10 && alt_frac >= 0.20) { tok_i++; // 6
      } } } } } } }

      //"-+123456"[tok_i]
      fwrite("-+123456" + tok_i, 1, 1, outfile);

      pos_cache.erase(curpos);

      // next position
      increment_pos(curchr, curpos);
   }

   // if read overlaps the current position, increment alt/refcounts along its span of the buffer
   if(cur_read.ChrID() == curchr &&
      cur_read.Position() <= curpos && 
      cur_read.PositionEnd() >= curpos) {

      // increment refcount along the whole span of the read in the buffer
      while((pos = curpos) < cur_read.PositionEnd()) pos_cache.increment(pos++, false);

      // if this read contains nonreference bases, we need to:
      // 1. increment altcount(s) at the relevant location(s)
      // 2. decrement refcount(s) at location(s)
      if(!EDz(cur_read)) {
	 uint32_t o = 0;
	 uint32_t pos;

	 vector<uint64_t> nrp = nonref_pos(cur_read);

	 for(auto& p : nrp) {
	    // sSNVs and insertions span single positions WRT reference
	    if(p >> 62 <= 1) {
	       poscache.increment(p & 0xFFFFFFFF, true);
	       poscache.decrement(p & 0xFFFFFFFF, false);

	    // otherwise, it's a deletion, which spans multiple positions WRT reference
	    } else {
	       for(uint16_t i = 0; i < p >> 48 & 0x3FFF; i++) {
		  poscache.increment(i + p & 0xFFFFFFFF, true);
		  poscache.decrement(i + p & 0xFFFFFFFF, false);
	       }
	    }
	 }
      }
   }
}

}
