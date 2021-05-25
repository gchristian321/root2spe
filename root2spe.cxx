#include <fstream>
#include <TH1.h>

struct SpeHeader {
   int32_t buffsize; /*fortran file, each record starts with record size */ // 14
   char    label[8];
   int32_t size;
   int32_t junk1;
   int32_t junk2;
   int32_t junk3;
   int32_t buffcheck; /*fortran file, record ends with record size :) */ // 14
} __attribute__((packed));


void WriteHist(TH1* hist, std::fstream* outfile)
{
   SpeHeader spehead;
   spehead.buffsize = 24;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
   strncpy(spehead.label, hist->GetName(), 8);
#pragma GCC diagnostic pop

   if(hist->GetRMS() > 16384 / 2) {
      while(hist->GetNbinsX() > 16384) {
         hist = hist->Rebin(2);
         printf("\t!!  %s has been compressed by 2.\n", hist->GetName());
      }
      spehead.size = hist->GetNbinsX();
   } else if(hist->GetNbinsX() > 16384) {
      spehead.size = 16384;
   } else {
      spehead.size = hist->GetNbinsX();
   }

   spehead.junk1     = 1;
   spehead.junk2     = 1;
   spehead.junk3     = 1;
   spehead.buffcheck = 24; /*fortran file, record ends with record size :) */ // 14

   outfile->write(reinterpret_cast<char*>(&spehead), sizeof(SpeHeader));

   int32_t histsizeinbytes = spehead.size * 4;

   outfile->write(reinterpret_cast<char*>(&histsizeinbytes), sizeof(int32_t));
   float bin = 0.0;
   for(int x = 1; x <= spehead.size; ++x) {
      if(x <= hist->GetNbinsX()) {
         bin = static_cast<float>(hist->GetBinContent(x));
         outfile->write(reinterpret_cast<char*>(&bin), sizeof(int32_t));
      } else {
         bin = 0.0;
         outfile->write(reinterpret_cast<char*>(&bin), sizeof(int32_t));
      }
   }

   outfile->write(reinterpret_cast<char*>(&histsizeinbytes), sizeof(int32_t));
}

void root2spe(TH1* hst, const char* filename)
{
	std::fstream ofs(filename, ios_base::out);
	WriteHist(hst,&ofs);
}
