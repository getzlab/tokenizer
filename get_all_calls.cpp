#include "get_all_calls.hpp"
#include "walker/argparse.hpp"

using namespace std;

namespace GAC {

bool gac_walker::walk_apply(const SeqLib::BamRecord& record) {
   // we are only interested in running on human primary contigs (chr1-Y)
   // TODO: make this flexible, so that tool can generalize to other organisms
   if(record.ChrID() > 23) return false;

   // advance to next read if read ends before current position
   // XXX: this might become a generic function in walker
   if(record.ChrID() < curchr) return true;
   if(record.ChrID() == curchr &&
      record.PositionEnd() < curpos) return true;

   // advance position(s) if read starts after current position
   while((record.Position() > curpos &&
	 record.ChrID() == curchr) ||
	 record.ChrID() > curchr) {

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

      fwrite("-+123456" + tok_i, 1, 1, outfile);

      pos_cache.erase(curpos);

      // next position
      increment_pos(curchr, curpos);
   }

   // if read overlaps the current position, increment alt/refcounts along its span of the buffer
   if(record.ChrID() == curchr &&
      record.Position() <= curpos && 
      record.PositionEnd() >= curpos) {

      uint32_t pos = curpos;

      // increment refcount along the whole span of the read in the buffer
      while(pos < record.PositionEnd()) pos_cache.increment(pos++, false);

      // if this read contains nonreference bases, we need to:
      // 1. increment altcount(s) at the relevant location(s)
      // 2. decrement refcount(s) at location(s)
      if(!EDz(record)) {
	 vector<uint64_t> nrp = nonref_pos(record);

	 for(auto& p : nrp) {
	    // sSNVs and insertions span single positions WRT reference
	    if(p >> 62 <= 1) {
	       pos_cache.increment(p & 0xFFFFFFFF, true);
	       pos_cache.decrement(p & 0xFFFFFFFF, false);

	    // otherwise, it's a deletion, which spans multiple positions WRT reference
	    } else {
	       for(uint16_t i = 0; i < (p >> 48 & 0x3FFF); i++) {
		  pos_cache.increment(i + (p & 0xFFFFFFFF), true);
		  pos_cache.decrement(i + (p & 0xFFFFFFFF), false);
	       }
	    }
	 }
      }
   }

   return true;
}

}

int main(int argc, char** argv) {
   walker::basic_arg_t args = {};
   args.ref_fa = "/home/jhess/j/db/hg19/ref/hs37d5.fa";
   if(!walker::basic_argparse(argc, argv, &args)) exit(1);
   if(!walker::basic_argparse_validate(&args)) exit(1);

   GAC::gac_walker w = GAC::gac_walker(args.bam_in, args.ref_fa);
   if(!w.set_output_file(args.output_file)) exit(1);

   w.walk();

   return 0;
}
