//-------------------------------------------------------------
// File: dist.cc
// Purpose: testing the distribution functions in OMNeT++
// Author: Andras Varga
//-------------------------------------------------------------

#include <string>
#include <ctype.h>
#include <string.h>
#include <omnetpp.h>

class Dist : public cSimpleModule
{
    public:
      Module_Class_Members(Dist,cSimpleModule,16384)
      virtual void activity();
};

Define_Module( Dist );

void Dist::activity()
{
    cPar& variate = par("variate");
    long n = par("n");
    bool discrete = par("discrete");
    int numcells = par("numcells");
    long firstvals = par("firstvals");
    const char *excel = par("excel");
    const char *filename = par("file");

    std::string distname = variate.getAsText();
    ev << "running: " << distname << endl;

    // generate histogram
    cHistogramBase *h;
    if (discrete)
        h = new cLongHistogram;
    else
        h = new cDoubleHistogram;
    h->setNumCells(numcells);
    h->setRangeAuto(firstvals,1);

    for (long i=0; i<n; i++)
    {
        double d = variate.doubleValue();
        h->collect(d);
    }

    // automatic filename
    char buf[500];
    if (filename[0]=='\0')
    {
        strcpy(buf, distname.c_str());
        for (char *s=buf; *s; s++)
           if (!isalnum(*s) && *s!='(' && *s!=')' && *s!=',' && *s!='-' && *s!='+')
               *s='_';
        strcat(buf, ".csv");
        filename = buf;
    }

    ev << "writing file: " << filename << endl;

    // write file
    FILE *f = fopen(filename, "w");
    fprintf(f,"\"x\",\"theoretical %s pdf\",\"measured %s\" pdf\n",distname, distname);

    for (int k=0; k<h->cells(); k++)
    {
        fprintf(f,"%lg,\"=",(h->basepoint(k)+h->basepoint(k+1))/2);
        fprintf(f,excel,k+2,k+2,k+2,k+2,k+2,k+2,k+2,k+2,k+2,k+2);
        fprintf(f,"\",%lg\n",h->cellPDF(k));
    }
    fclose(f);

    // next line is for Tkenv, so that one can inspect the histogram
    wait(1.0);

    delete h;
}



