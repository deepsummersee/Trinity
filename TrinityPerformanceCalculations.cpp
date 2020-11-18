#include <TGraph.h>
#include <TGaxis.h>
#include <TTimer.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TStyle.h>
#include <TMath.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TMarker.h>
#include <TText.h>

#include <TROOT.h>
#include <TApplication.h>


#include <stdio.h>
#include <fstream>
#include <iostream>

using namespace std;

Double_t sigma[] = { 660, 920, 1400, 1900, 2500, 3700, 4800, 6200, 8700, 11000, 14000, 19000, 24000, 30000, 39000, 48000, 59000, 75000 }; //pb cc cross section cooper-sarkar 2011
Double_t sigmaNC[] = { 240, 350, 530, 730, 980, 1400, 1900, 2400, 3400, 4400, 5600, 7600, 9600, 12000, 16000, 20000, 24000, 31000 }; //pb
Double_t Esig[] = { 1e6, 2e6, 5e6, 1e7, 2e7, 5e7, 1e8, 2e8, 5e8, 1e9, 2e9, 5e9, 1e10, 2e10, 5e10, 1e11, 2e11, 5e11 }; //GeV

//make plot of length distribution of showers
//make plot of azimuth and elevation distribution as function of distance and wi
//need good estimate of length of shower


//Ghandi 1996
//Double_t sigma[] = { 634, 960, 1412, 1749, 2554, 3630, 4436, 6283, 8700, 10490, 14660, 20100, 23790, 32890, 44270, 53570, 73200, 99270, 117900 }; //pb
//Double_t sigmaNC[] = { 260, 402, 600, 748, 1104, 1581, 1939, 2763, 3837, 4641, 6490, 8931, 10660, 14650, 19950, 23770, 32470, 43770, 51960 }; //pb
//Double_t Esig[] = { 1e6, 2.5e6, 6e6, 1e7, 2.5e7, 6e7, 1e8, 2.5e8, 6e8, 1e9, 2.5e9, 6e9, 1e10, 2.5e10, 6e10, 1e11, 2.5e11, 6e11, 1e12 }; //GeV


int iColors[] = {kBlue-3,kCyan-3,kGreen-3,kYellow-3,kRed-3,kRed+3,kMagenta-3};
int marker[] = { 23, 22, 29, 21, 20, 28, 25};
double markerSize[] = { 0.9, 0.9, 1.2, 0.7, 0.9, 1.3, 0.9};

TF1 *fPE = 0;

Double_t c = 299792; //km/s
Double_t pi = 3.14159265359;
Double_t DecayTime = 0.290e-12; //s
Double_t Mtau = 1.7768; //GeV
Double_t REarth = 6371; //km

//All the coefficients to get the right PE intensity
///////////////////////////////

Int_t iConfig = 2;
double DetectorAltitude[] = { 0, 1, 2, 3};
 
//obtained from 3e4 GeV gamma rays
double lincorr[] = { 0.0509251, 0.0522854, 0.0595455, 0.0642221};
double scalefirst[] = { 1.00001, 1.03346, 1.53535, 2.36961};
double eleScaling[] = { 0.00163848, 0.00191408, 0.0071185, 0.0513182};
double absorptionlength[] = { 16.7049, 16.6106, 17.9808, 19.0274};
//Parameterization of PE distribution at 50km, 0ele, and 0 Altitude
//par[0]*exp(-xx/par[1])+par[2]*exp(-xx/(par[3]+xx*par[4]))
double parPEF[] = { 0.000332267, 0.580756, 4.25751e-05, 1.9491, 0.0427249};

/*
//obtained from 1e6 GeV gamma rays
double lincorr[] = { 0.0497124, 0.0493805, 0.0552341, 0.0604423};
double scalefirst[] = { 1.00001, 1.02504, 1.42546, 1.73371};
double eleScaling[] = { 0.00143344, 0.00164536, 0.00419904, 0.00479111};
double absorptionlength[] = { 16.7647, 16.8572, 18.3202, 19.4597};
//Parameterization of PE distribution at 50km, 0ele, and 0 Altitude
//par[0]*exp(-xx/par[1])+par[2]*exp(-xx/(par[3]+xx*par[4]))
double parPEF[] = { 0.00038827, 0.555588, 4.66631e-05, 1.90266, 0.0426453};
*/
///////////////////////

Double_t dMinEnu = 8.5;
Double_t dMaxEnu = 9.5;
Double_t dST = 10; //km max height of shower tip above ground;
Double_t yMin = 1;
Double_t yMax = 60;
Double_t yDelta = 1;
Double_t MaxElevation = 10; //elevation angle (determines path through Earth;
Double_t DeltaAngle = 0.1; //steps in azimuth and elevation 
Double_t DeltaAngleAz = 0.1; //steps in azimuth  
Double_t nuIndex = 2.2; //power law index of the neutrino spectrum the minus sign is added later
Double_t dMaxCherenkovAzimuthAngle = 40.0; //maximum azimuth angle for cherenkov 
Double_t dMaxFluorescenceDistance = 100;

//next three parameters are key to the instrument
//plus the telescop height above ground which can be selected above
Double_t tanFoV = tan(5/180.*pi); //Field of view of telescope above the horizon
Double_t dFoVBelow = 5/180.*pi; //Field of view of telescope  below horizon 
Double_t dMinLength = 0.3; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.
Double_t dMinimumNumberPhotoelectrons = 20; 

Int_t iMirrorSize = 1;
Double_t dMirrorA[] = {1.0, 5.0, 10.0, 100.0}; //m^2 
//Double_t dThreshold[] = {8*3, 19*3, 22*3, 120*3}; //pe //three fold coincidence. 
//Double_t dThreshold[] = {10*2, 22*2, 24*2, 155*2}; //pe //two fold coincidence
Double_t dThreshold[] = {10*2, 22*2, 24*2, 155*2}; //pe //two fold coincidence

Bool_t bFluorescence = kFALSE;
Bool_t bCombined = kFALSE;
Bool_t bMonoNu = kFALSE; //simulate monoenergetic neutrinos, only good for acceptance calculation. For all other simulations set it to kFALSE
TGraph *grsCC;
TGraph *grsNC;

TH1D *hTriggeredAzimuthAngles;

//~ TH2F *skymap = new TH2F("skymap","Acceptance Skymap", 360, -180, 180, 360, -180, 180);
//~ TH2F *skymap = new TH2F("skymap","Acceptance Skymap", 360, -180, 180, 179, -89.5, 89.5);
//~ TH2F *skymapSingleAngle = new TH2F("skymapSingleAngle","Acceptance Skymap of 360 Degree Airshower Azimuth Sweep [150 km, 10^9 GeV]", 3601, -180.05, 180.05, 1801, -90.05, 90.05);
//~ TH2F *skymapFull360Sweep = (TH2F*)skymapSingleAngle->Clone("skymapFull360Sweep");
//~ TH2F *skymapFullProjection = new TH2F("skymapFullProjection","360 FoV Projection In Galactic Coordinates Over 10 Years [10^9 GeV]", 361, -180.05, 180.05, 181, -90.05, 90.05);
//~ TH2F *skymapFullProjection = new TH2F("skymapFullProjection","360 FoV Projection over 12 Hour Exposure", 3601, -180.05, 180.05, 1801, -90.05, 90.05);
//~ TH2F *skymapTEMP = (TH2F*)skymapSingleAngle->Clone("skymapFullProjection");

Double_t latitude;
Double_t tStep;
//~ Double_t y;
Double_t MaxAzimuth;
Bool_t multNorm = kTRUE;

string Hold()
{
  string input;

  //hold the code;
  TTimer timer("gSystem->ProcessEvents();", 50, kFALSE);
  timer.TurnOn();
  cout<<"Press Enter to continue:"<<endl;
  getline(cin,input);
  timer.TurnOff();
  return input;
  
}

Double_t myPEfunction(Double_t *x, Double_t *par)
{
   //par[0] Distance to where tau comes out in km
   //par[1] Elevation in rad
   //azimuth angle is our x
   Float_t xx =x[0]; //angle is rad here 
   //if angle is larger 40 degrees return 0
   if(xx>0.69813170)
     return 0;

   //Calculate azimuth angle in frame of master pe distribution (50km, 0ele,
   //0altitude
   Double_t dTelAngle = atan(DetectorAltitude[iConfig]*1e-3/par[0]);
   Double_t dAngle = sqrt(xx*xx + (par[1]-dTelAngle)*(par[1]-dTelAngle))*57.295780; //in deg
   //calculate how many PEs / per m2 per GeV
   Double_t f = 0;
   if(dAngle<1.3)
     f = parPEF[0]*exp(-1.1/parPEF[1])+parPEF[2]*exp(-1.1/(parPEF[3]+1.1*parPEF[4]));
   else
     f = parPEF[0]*exp(-dAngle/parPEF[1])+parPEF[2]*exp(-dAngle/(parPEF[3]+dAngle*parPEF[4]));

//other parameters to get PE intensity for different distance, azimuth and
//elevation
   //scale PE distribution to first PE distribution at 50km distance
   f*=   scalefirst[iConfig];
   //Get elevation dependence
   f*= (2-exp(-par[1]/eleScaling[iConfig]));
   //Get Distance dependence
   f*=  exp(-(par[0]-55)/
           (absorptionlength[iConfig]+(par[0]-55)*lincorr[iConfig])); //55km is the distance for which the normalized PE distribution is extracted

   return f;
}

  

Double_t DistanceThroughEarth(Double_t y, Double_t elevation, Double_t azimuth)
{

  elevation = elevation/180*pi; //elevation angle (determines path through Earth;
  azimuth = azimuth/180.*pi;  //azimuth angle

  Double_t l = y; //Distance from detector to where the tau comes out detector is always at z=0

  Double_t v = sqrt((REarth+DetectorAltitude[iConfig])*(REarth+DetectorAltitude[iConfig])-REarth*REarth);

  //shortest distance d between tau trajectory and detector
  Double_t nproj = y*sqrt( 1 + tan(azimuth)*tan(azimuth) ); //projection of trajectory to x-y plane
  Double_t denomsquared= y*tan(azimuth)*y*tan(azimuth) + y*y + nproj*nproj*tan(elevation)*tan(elevation)  ;
  
  //normalized trajectory vector of tau
  Double_t dNormalize = y/sqrt(denomsquared);
  //Double_t dNx = dNormalize * tan(azimuth);
  Double_t dNy = -dNormalize;
  Double_t dNz = dNormalize * sqrt( 1 + tan(azimuth)*tan(azimuth) ) * tan(elevation);


  Double_t p = 2 * ( REarth*dNz - (v-l)*dNy );
  Double_t q = (v-l)*(v-l);

  if(q-p*p/4>=0) //trajectory does not intersect with Earth
      return 0;
        
  Double_t i1 = p/2. - sqrt(p*p/4.-q);
  Double_t i2 = p/2. + sqrt(p*p/4.-q);

return fabs(i2-i1);

}


string star ;
double number;
int distanceNumber;
vector<double> enerNu,enerTau,prob,dist,EtauNorm;
unsigned rem = 0 ;
void removeDuplicates()
{
	while(rem < enerNu.size()-1){
		if(enerNu[rem]==enerNu[rem+1]){
			enerNu.erase(enerNu.begin()+rem+1);
			removeDuplicates();
		}else if(enerNu[rem+1]==enerNu[rem+2]){
			rem ++;
			removeDuplicates();
		}
	}
}
void readFromTable(){
	ifstream ifs("table_with_e_05_a_1.txt") ;
	if(ifs.is_open()){
	ifs>>star;
		while(ifs.good()){
			ifs>>number;
			enerNu.push_back( pow(10,number-9.0) );
			ifs>>number;
			//angle.push_back(number);
			dist.push_back(cos((180-number)*pi/180)*2*REarth);
			for(int i=0;i<100;i++){
				ifs>>number;
				enerTau.push_back( pow(10,number-9.0) );
                                EtauNorm.push_back( pow(10,4+i*0.07) );
				ifs>>number;
				prob.push_back(number);
			}
			ifs>>number;
			enerTau.push_back( pow(10,number-9.0) );
                        EtauNorm.push_back( pow(10,4+100*0.07) );
			ifs>>star;
		}
		cout << "size: " << enerNu.size() << endl;
		removeDuplicates() ;
	}
}
void findAngleNumber(){
	for(unsigned i=1;i<enerNu.size();i++){
		if(dist[0]==dist[i]){
			distanceNumber = i;
			break;
		}else{
			continue ;
		}
	}
}
double biLinearInterpolation(double a1,double n1,double q11,double a2,double n2,double q22,double x,double y){
	double q12 = (q11 + q22)/2 ;
	double q21 = q12 ;
	double p = (n2-y)/(n2-n1)*( (a2-x)/(a2-a1)*q11 + (x-a1)/(a2-a1)*q21 ) + (y-n1)/(n2-n1)*( (a2-x)/(a2-a1)*q12 + (x-a1)/(a2-a1)*q22 ) ;

return p ;
}

//Find index in lookuptable
int FindLion(double dValue, vector<double> &vData,int iSize)
{
          int iWidth = iSize/2;
          int index = iWidth;
          //cout<<dValue<<": ";
          while(iWidth>1 && vData[index]!=dValue && index > 0  && index <iSize)
            {
              iWidth = iWidth/2+ iWidth%2;
              index = vData[index]<dValue ? index + iWidth :  index - iWidth;
              //cout<<index<<" "<<iWidth<<"  "<<vData[index]<<"; ";
            }
          while((dValue>vData[index] || index<0) && index < iSize)
             index++;
          index--;

          if(dValue==vData[index] || index>=iSize)
             index--;

          if(index<0)
            index=0; 
 
         //cout<<index<<endl;

return index;
}
//Calculates the probability of tau emergence using NuTauSim LUT
Double_t PEtau(Double_t D,Double_t Etau, Double_t Enu, TH1D *hTau) 
{
	//Enu = log10(Enu) + 9.0;
	//Etau = log10(Etau) + 9.0 ;
	//double zenithAngle = 180 - acos(D/2/REarth)/M_PI*180 ;
//	if(zenithAngle >= angle[0] && zenithAngle <= angle[angleNumber-1] && Enu>=enerNu[0] && Enu <=enerNu[enerNu.size()-1]){
	if(D >= dist[0] && D <= dist[distanceNumber-1] && Enu>=enerNu[0] && Enu <=enerNu[enerNu.size()-1] && Etau>=enerTau[0] && Etau <=enerTau[100]){

         int indexEnu = FindLion(Enu,enerNu,enerNu.size());

	 int indexDistance = FindLion(D,dist,distanceNumber);

	 int indexEtau = FindLion(Etau,enerTau,100);
		
                int indexProb1 = indexEnu*distanceNumber*100+indexDistance*100+indexEtau;
		double p1 = prob[indexProb1] ;
		int indexProb2 = (indexEnu+1)*distanceNumber*100 + (indexDistance+1)*100 + indexEtau ;
		double p2 = prob[indexProb2] ;

		double Prob = biLinearInterpolation(dist[indexDistance],enerNu[indexEnu],p1,dist[indexDistance+1],enerNu[indexEnu+1],p2,D,Enu)/
						(EtauNorm[indexEtau+1]-EtauNorm[indexEtau]);
						//(pow(10,4+(indexEtau+1)*0.07)-pow(10,4+indexEtau*0.07));
		//cout<<Prob<<endl;
		return Prob ;
	}else{
		return 0 ;
	}
}



//Probability that Tau with Energy Etau emerges for initial nu energy Enu
//Thickness of matter d 
//Does not use energy loss of tau in Earth.
//Assumes the energy of the tau is 0.8*Enu
//Double_t PEtauNoTauEnergyLoss(Double_t D,Double_t Etau, Double_t Enu, TH1D *hTau) 
//Double_t PEtauNoTauEnergyLoss(Double_t D,Double_t Etau, Double_t Enu, TH1D *hTau) 
Double_t PEtauNoEnergyLoss(Double_t D,Double_t Etau, Double_t Enu, TH1D *hTau) 
{

int n = hTau->FindBin(Etau);
if(hTau->GetBinLowEdge(n)>0.8*Enu  || hTau->GetBinLowEdge(n+1)<0.8*Enu )
  return 0;

Double_t sCC = grsCC->Eval(Enu); //crossection in pB
Double_t sNC = grsNC->Eval(Enu); //crossection in pB
Double_t rho = 2.65; //density in g/cm3
Double_t NA = 6.022142e23;

Double_t dInvConvCC = sCC*rho*NA*1e-31; // 1/km 1e-12*1e-28*1e4*1e5
Double_t db = (sNC+sCC)*rho*NA*1e-31; // 1/km
Double_t da = Mtau/(DecayTime*c*0.8*Enu); //1/km

//cout<<dInvConvCC<<endl;
//cout<<"neutrino interaction: "<<db<<" "<<exp(-1.0*db*D)<<endl;
//cout<<"tau survival: "<<da<<"  "<<exp(-1.0*da*D)<<endl;

Double_t Prob = dInvConvCC / (da-db) * ( exp(-1.0*db*D)-exp(-1.0*da*D) );  

if(Prob<0)
  return 0;

Prob /= (hTau->GetBinLowEdge(n+1)-hTau->GetBinLowEdge(n));

return Prob;  
}

//Probability that Tau with Energy Etau emerges for initial nu energy Enu
//Thickness of matter d 
//follows description in Dutta 2005 in particular equation 28 with
//parameterization of beta in equation 13 case II
//energies in GeV distances in km at inptut
//Fails <1e8 GeV because energy loss (Beta) becoms <0
Double_t PEtauDutta(Double_t D,Double_t Etau, Double_t Enu,TH1D *hTau) 
{

if(Etau>0.8*Enu)
  return 0;

Double_t sCC = grsCC->Eval(Enu); //crossection in pB
Double_t sNC = grsNC->Eval(Enu); //crossection in pB
Double_t rho = 2.65; //density in g/cm3
Double_t NA = 6.022142e23;

Double_t dInvConvCC = sCC*rho*NA*1e-31; // 1/km 1e-12*1e-28*1e4*1e5
Double_t dInvConvtotal = (sNC+sCC)*rho*NA*1e-31; // 1/km
Double_t beta = 1.2e-6 + 0.16e-6 * log(Etau/1e10); //cm2/g Equation 13 case II in Dutta 
//cout<<"beta "<<beta<<endl;
Double_t prefactor = Mtau/(DecayTime*c*1e5*beta*rho*Etau); //dimensionless
//cout<<"Prefactor: "<<prefactor<<endl;

Double_t xDelta = log(Etau/(0.8*Enu))/(beta*rho)*1e-5+D; //where delta function is non zero; 1e-5 convert from cm to km 

if(xDelta<0)
  return 0;

//cout<<"beta: "<<beta<<" rho: "<<rho<<" Etau: "<<Etau<<endl;
Double_t Prob = 1.e-5/(beta*rho*Etau); //km/GeV
//cout<<"Prob: "<<Prob<<endl;
Double_t Pnu = dInvConvCC*exp(-xDelta*dInvConvtotal);  // 1/km
//cout<<"Pnu: "<<Pnu<<endl;
Prob *= Pnu; 
//cout<<"Prob2: "<<Prob<<endl;
if(Prob<0)
  return 0;

if(prefactor<0)
  return 0;
  
Double_t Ptau = exp(-prefactor*(1.0-exp(-beta*rho*(D-xDelta)*1e5))); //1e5 to convert from km to cm 
//cout<<"Ptau: "<<Ptau<<endl;
//cout<<1.0-exp(-beta*rho*(D-xDelta)*1e5)<<endl;
Prob *= Ptau;

return Prob;  
}

Double_t PDecayFluorescence(Double_t Etau, Double_t y, Double_t elevation, Double_t azimuth)
{
  //cout<<endl<<"elevation: "<<elevation<<" azimuth: "<<azimuth<<" distance: "<<y<<endl;
  elevation = elevation/180*pi; //elevation angle (determines path through Earth;
  azimuth = azimuth/180.*pi;  //azimuth angle

  Double_t l = y; //Distance from detector to where the tau comes out detector is always at z=0

  //shortest distance d between tau trajectory and detector
  Double_t nproj = y*sqrt( 1 + tan(azimuth)*tan(azimuth) ); //projection of trajectory to x-y plane
  Double_t denomsquared= y*tan(azimuth)*y*tan(azimuth) + y*y + nproj*nproj*tan(elevation)*tan(elevation)  ;
  
  //normalized trajectory vector of tau
  Double_t dNormalize = y/sqrt(denomsquared);
  Double_t dNx = dNormalize * tan(azimuth);
  Double_t dNy = -dNormalize;
  Double_t dNz = dNormalize * sqrt( 1 + tan(azimuth)*tan(azimuth) ) * tan(elevation);

  if(azimuth>=pi/2.)
    {
      dNx*=-1;
      dNy*=-1;
    }

  //cout<<"trajectory vector normalized x: "<<dNx<<" y: "<<dNy<<" z: "<<dNz<<" normalization: "<<dNormalize<<endl;

  //crossproduct of trajectory vector and vector of where tau emerges. Gives the
  //distance between the to perpendicular to the trajecotory vector
  //Double_t dx = y*dNz; 
  //Double_t dy = 0;
  //Double_t dz = y*dNx;

  //Double_t d = sqrt(dx*dx+dy*dy+dz*dz); //shortest distance d between tau trajectory and detector

  //Double_t dem = sqrt(l*l-d*d);

  //maximum length of trajectory above horizon befor track leaves atmosphere (dST above ground)
  //calculation is not entirely correct but we do not max out on this distance
  //anyway
  Double_t v = sqrt((REarth+DetectorAltitude[iConfig])*(REarth+DetectorAltitude[iConfig])-REarth*REarth);

  Double_t phi = elevation + asin( REarth/sqrt(REarth*REarth+(l-v)*(l-v)) ); //if azimuth > 90

  Double_t alpha = asin( sin(phi) * sqrt(REarth*REarth+(l-v)*(l-v)) / (REarth+dST)  );

  Double_t gamma = pi - alpha - phi;

  Double_t dMaxDist = (REarth+dST) *  sin(gamma)/sin(phi);


 //trim the path length to be fully inside the atmosphere, should always be the
 //case
  Double_t dED = 0; //extra distance
  Double_t dInFoV=0; //Distance in between lower edge of FoV and line of sight to horizon

  //If the camera FoV below the horizon is larger than the maximum angle needed to
  //cover all the solid angle below the horizon, set the FoV below the horizon
  //to the maximum angle needed.
  Double_t dMaxFoVBelow = asin(REarth/(REarth+DetectorAltitude[iConfig]));
  if(dFoVBelow>dMaxFoVBelow)
      dFoVBelow=dMaxFoVBelow; 


  //length of the visible trajectory between the plane to horizon and the lower edge of the camera
  //FoV below the horizon
  dInFoV = l * sin(dFoVBelow) / ( dNy * sin(dFoVBelow) +  dNz * cos(dFoVBelow)  );

  //length of tau trajectory below the horizon and earth surface
  Double_t p = 2 * ( REarth*dNz - (v-l)*dNy );
  Double_t q = (v-l)*(v-l);

  if(q-p*p/4>=0) //trajectory does not intersect with Earth
      return 0;
        
  Double_t i1 = p/2. - sqrt(p*p/4.-q);
  Double_t i2 = p/2. + sqrt(p*p/4.-q);
            
  if(i1<0 || i2<0)
    cout<<"PDecay: i1 or i2 less than 0: "<<i1<<"  "<<i2<<endl;

  dED= i1<i2 ? i1 : i2;

  if( (dInFoV>dED && dInFoV>0 ) || dInFoV<0)
          dInFoV=dED;
 
  if(l>v) //if shower emerges beyond horizon
     dInFoV = 0;


  dMaxDist = dST/sin(elevation) + dInFoV; //takes into account distance visible below plane to horizon
 // cout<<"dMaxDist: "<<dMaxDist<<endl;

  Double_t dTermInSquareRoot = cos(elevation)*cos(azimuth)*cos(elevation)*cos(azimuth)
                               + sin(elevation)*sin(elevation)/tanFoV/tanFoV
                               - cos(elevation)*cos(elevation);

  if(dTermInSquareRoot>0) //if it is negative the shower is contained in the FoV anywhere along the track
    {
        //maximum trajectory length before the tip of the shower is not contained in the camera anymore
       Double_t dMaxDisttoSatisfyFovReq = l / ( cos(elevation)*cos(azimuth) + sqrt( dTermInSquareRoot ));
       //if the value is negative, the elevation angle is less than the Max FoV would have to point below the
       //horizon
       if(dMaxDist>dMaxDisttoSatisfyFovReq+dInFoV && dMaxDisttoSatisfyFovReq>0) //correct max. trajectory length
           dMaxDist = dMaxDisttoSatisfyFovReq+dInFoV;
   //   cout<<" dMaxDisttoSatisfyFovReq: "<<dMaxDisttoSatisfyFovReq<<endl;
    }

  
  //length of shower in camera plane
  Double_t dShwrLgth = 0.304 * log(Etau*0.5/0.088)/log(2); //0.304km radiation length at see level, 0.088GeV critical energy of electrons in air,only 0.5 of the energy goes into the electromagnetic shower
  //cout<<"Etau "<<Etau<<" size of shower in km: "<<dShwrLgth<<endl;
  //cout<<" dMaxDist: "<<dMaxDist<<endl;
  
  //ok have taken all requirements into account. Lets see if the trajectory
  //length allows for a full shower development. If not we quit.
  if(dMaxDist<dShwrLgth)
   return 0;


  //make sure the shower does not develop past the point where more than 90% of the
  //taus have decayed
  Double_t DecayLength = Etau * c * DecayTime / Mtau;
  Double_t d90PctDecayLength = -log(0.1)*DecayLength;
  //cout<<"90% of taus decayed after: "<<d90PctDecayLength<<endl;

  if(d90PctDecayLength+dShwrLgth<dMaxDist)
    dMaxDist = d90PctDecayLength+dShwrLgth;

  //cout<<" dMaxDist: "<<dMaxDist<<endl;
  //calculate how far away from the detector the shower can be to be still
  //detected
  //emitted light intensity
  Double_t dLight = 5.95844e3*Etau; //in photons. 5.958 comes from the macro FluorescenceDetectionYield.C and includes PDE of S14520-6050CN, it is the integral from 300 to 430nm 
  dLight /= 4 * pi; //so we do not have to do it in every loop below
  //and absorption 0.9 at 337nm is used in condition below
  Double_t dFluorescenceMaximumDistance = 10; //km

  while(1)
   {
      //the 1e-6 is for a 1m^2 mirror
      if(dLight*1e-6/dFluorescenceMaximumDistance/dFluorescenceMaximumDistance*exp(dFluorescenceMaximumDistance*log(0.9))>dMinimumNumberPhotoelectrons)
      dFluorescenceMaximumDistance++; 
     else
      break;
     //cout<<dFluorescenceMaximumDistance<<"   "<<dLight*1e-6/dFluorescenceMaximumDistance/dFluorescenceMaximumDistance*exp(dFluorescenceMaximumDistance*log(0.9))<<endl;

   }  
  dFluorescenceMaximumDistance--;
  //cout<<"Maximum Distance between Detector and Shower: "<<dFluorescenceMaximumDistance<<endl;
  //don't know if this is actually good
 
  //check if we need to increase limit 
  if(dMaxDist>dFluorescenceMaximumDistance) //too speed up calculations
    dMaxDist=dFluorescenceMaximumDistance;
  

  //now lets check if the size of the shower is fullfilling the minimum length
  //requirement

  //calculating length of shower in the camera assuming the shower happens late
  //and develops up to the maximumg possible point along the trajectory

  Double_t dLength = 0.0;
  Double_t m = dMaxDist;
  Double_t B = sqrt( m* dNx * m* dNx + (m*dNy+y) * (m*dNy+y) + m*dNz * m*dNz  );
  Double_t A = 0.0;


  //This is if the shower passed and develops behind
  while(dMaxDist>dShwrLgth && (dLength<dMinLength || B>dFluorescenceMaximumDistance ))
   {
     Double_t n = dMaxDist - dShwrLgth -dInFoV;
              m = dMaxDist;

     Double_t A = sqrt( n* dNx * n* dNx + (n*dNy+y) * (n*dNy+y) + n*dNz * n*dNz  );
              B = sqrt( m* dNx * m* dNx + (m*dNy+y) * (m*dNy+y) + m*dNz * m*dNz  );

     Double_t costheta = n*dNx * m*dNx + (n*dNy+y)*(m*dNy+y) + m*dNz * n*dNz;
     costheta = costheta / (A*B);
     dLength = acos(costheta)*180/pi;
 //    cout<<"l"<<l<<"az"<<azimuth*180/pi<<"size of shower in degrees: "<<dLength<<" cos of angle:  "<<costheta<<" dMaxDist "<<dMaxDist<<"dShwrLgth "<<dShwrLgth<<" minimum shower length in deg  "<<dMinLength<<" Distance of tip of shower to telescope  "<<B<<endl;
     if((dLength<dMinLength && B>l) || B>dFluorescenceMaximumDistance )
       dMaxDist-=1.0;
     else
       break;
   }
  dMaxDist+=1.0;

  if(dLength<dMinLength  || B>dFluorescenceMaximumDistance || A>dFluorescenceMaximumDistance) //shower is not long enough in the camera or either end of the shower does not produce sufficient intensity in the telescope
      return 0;

//  need to check if the shower is pointing away from the telescope and if we find a distance in which the tau can decay and develop a shower which appears larger. Need to adjust the distance so the image has the minimal required size.

  //Ok finally we are there. Lets decay the tau in the remaining distance we
  //have
 
  Double_t ProbTauDecay = exp(-(dED-dInFoV)/DecayLength); //Tau has to to survive before it becomes visible to the detector

  ProbTauDecay *= 1-exp(-(dMaxDist-dShwrLgth)/DecayLength); // then it has to decay before it is out of the FoV

  ProbTauDecay*=0.8;//only 80% of taus make a shower

//if(azimuth*180/pi>90 && azimuth*180/pi<8.1)
//cout<<"Etau: "<<Etau<<" el: "<<elevation*180/pi<<" az: "<<azimuth*180/pi<<" alpha "<<alpha<<" beta "<<180/pi*(pi - asin(sinbeta))<<" Prob: "<<ProbTauDecay<<" l "<<l<<" v "<<v<<" below horizon: "<<dBH<<" MaxDistOfTrack: "<<dMaxDist<<" minimal distance between trajectory and telescope "<<d<<endl;
return ProbTauDecay;
}



Double_t PDecay(Double_t Etau, Double_t y, Double_t elevation, Double_t azimuth)
{
  elevation = elevation/180*pi; //elevation angle (determines path through Earth;
  azimuth = azimuth/180.*pi;  //azimuth angle

  Double_t l = y; //Distance between the detector and the point where the tau emerges from the ground. The detector is always at z=0
  //Distance between telescope and horizon
  Double_t v = sqrt((REarth+DetectorAltitude[iConfig])*(REarth+DetectorAltitude[iConfig])-REarth*REarth);

  //Below: the shortest distance d between tau trajectory and detector d
  Double_t nproj = y*sqrt( 1 + tan(azimuth)*tan(azimuth) ); //projection of trajectory to x-y plane
  Double_t denomsquared= y*tan(azimuth)*y*tan(azimuth) + y*y + nproj*nproj*tan(elevation)*tan(elevation)  ;
  
  //normalized trajectory vector of tau
  Double_t dNormalize = y/sqrt(denomsquared);
  Double_t dNx = dNormalize * tan(azimuth);
  Double_t dNy = -dNormalize;
  Double_t dNz = dNormalize * sqrt( 1 + tan(azimuth)*tan(azimuth) ) * tan(elevation);

  //cout<<"trajectory vector normalized x: "<<dNx<<" y: "<<dNy<<" z: "<<dNz<<" normalization: "<<dNormalize<<endl;

  //crossproduct of the trajectory vector with the vector pointing to where the tau emerged from the ground. 
  //The magnitude of the cross product gives the distance of closest approach of the tau to the detector as it travels along 
  //its trajectory
  Double_t dx = y*dNz; 
  Double_t dy = 0;
  Double_t dz = y*dNx;

  Double_t d = sqrt(dx*dx+dy*dy+dz*dz); //shortest distance d between tau trajectory and detector

  //add some extra distance if the tau passes plane in between telescope and
  //horizon
  //using dED term assumes we see shower in camera but lower edge of FoV is aligned with line of sight to the horizon
  Double_t dED = 0; //extra distance
  Double_t dInFoV=0; //Distance in between lower edge of FoV and line of sight to horizon

  //Reset FoV below to maximum possible if it is larger
  Double_t dMaxFoVBelow = asin(REarth/(REarth+DetectorAltitude[iConfig]));
  if(dFoVBelow>dMaxFoVBelow)
      dFoVBelow=dMaxFoVBelow; 


  //length of trajectory between plane to horizon and lower edge of camera
  //FoV
  dInFoV = l * sin(dFoVBelow) / ( dNy * sin(dFoVBelow) +  dNz * cos(dFoVBelow)  );

  //length of trajectory between plane to horizon and earth surface
  Double_t p = 2 * ( REarth*dNz - (v-l)*dNy );
  Double_t q = (v-l)*(v-l);

  if(q-p*p/4>=0) //trajectory does not intersect with Earth
      return 0;
        
  Double_t i1 = p/2. - sqrt(p*p/4.-q);
  Double_t i2 = p/2. + sqrt(p*p/4.-q);
            
  if(i1<0 || i2<0)
    cout<<"PDecay: i1 or i2 less than 0: "<<i1<<"  "<<i2<<endl;

  dED= i1<i2 ? i1 : i2;

  if( (dInFoV>dED && dInFoV>0 ) || dInFoV<0)
          dInFoV=dED;
 
  if(l>v) //if shower emerges beyond horizon
     dInFoV = 0;

  //maximum distance from where tau emerges to that plane
  //modify this to take into account extra length if shower emerges before the
  //horizon
  //add extra length if trajectory crosses plane between telescope and horizon
  Double_t dem = sqrt(l*l-d*d) + dInFoV;


  //fix this to use elevation measured when shower emerges from ground
  fPE->FixParameter(1,elevation ); //Shower elevation in rad

  //length of shower in camera plane
  Double_t dShwrLgth = 0.304 * log(Etau*0.5/0.088)/log(2); //0.304km radiation length at see level, 0.088GeV critical energy of electrons in air,only 0.5 of the energy goes into the electromagnetic shower
  //cout<<"Etau "<<Etau<<" size of shower in km: "<<dShwrLgth<<endl;
  //cout<<" dMaxDist: "<<dMaxDist<<endl;

  //minimal distance from tip of shower to the plane that is normal to trajectory and goes through the origin (where the detector is located), constrained by maximum angle a sufficient Cherenkov light reaches the detector. values below are 90-a and a in the sines. Need to have functions of tau energy and distance for a 
  //dd is first used as the distance to the start of the shower not hte tip of
  //the shower
  Double_t dd = dShwrLgth+5; //we need to at least have the shower develop. note we neglect the decay length here, which does not really matter. That is taken care of later.
 
  if(dd>dem)
     dd = dShwrLgth;
 
  while(dd<dem)
   {
  //move along trajectory and find spot where MaxCherenkovAngle condition is
  //fullfilled
  Double_t dDistanceToWhereTauStarts = sqrt(d*d+dd*dd);
  fPE->FixParameter(0,dDistanceToWhereTauStarts); //Distance to where the tau starts shower
   //get new azimuth
   Double_t g = (dem-dd) * cos(elevation);
   Double_t az = 0;
   if(g>0 && azimuth>0)
     {
        Double_t xi = ( l - g * cos(azimuth)) / (g * sin(azimuth));
        az =  pi*0.5 + azimuth - atan(xi);
        
        if (dem-dd-dInFoV < 0 ) //if we are below the plane
          az = azimuth;
     }
    //~ else
      //~ cout<<"in PDecay, azimuth is zero or g is smaller zero"<<endl;
   //get PE for new azimuth
     //double az = asin(d/dDistanceToWhereTauStarts); //azimuth for that distance
   if(fPE->Eval(az)*Etau*0.5<dMinimumNumberPhotoelectrons)
       dd++;
     else
      break;

   }

  //cout<<"maximum available length for decay and shower to happen (dem:) "<<dem<<" distance between trajectory and telescope d: "<<d<<" dd: "<<dd<<endl;
  //tip of the shower has to be inside the atmosphere. Check if that is the case. if not adjust dd
  //cout<<"need to takeaway dd so we can see all the Cherenkov light: "<<dd<<endl; 
  //if dem is less then dd, which means Cherenkov light will not hit the
  //telescope. return 0
  if(dd>dem) // the shower cannot be seen by the telescope because the cherenkov cone does not illuminate the telescope anywhere along the track
   return 0;

  //dd below is used as the distence between the plane perp. to the trajectory
  //and the tip of the shower so lets subtract the length of the shower and the
  //5 km again
  if(dd<dShwrLgth+5)
    dd -= dShwrLgth;
  else
    dd -= (dShwrLgth+5);
  
  
  //maximum length of trajectory above horizon befor track leaves atmosphere (dST above ground)
  Double_t phi = elevation + asin( REarth/sqrt(REarth*REarth+(l-v)*(l-v)) );
  
  Double_t alpha = asin( sin(phi) * sqrt(REarth*REarth+(l-v)*(l-v)) / (REarth+dST)  );

  Double_t gamma = pi - alpha - phi;

  Double_t dMaxDist = (REarth+dST) *  sin(gamma)/sin(phi);

 //trim the path length to be fully inside the atmosphere, should always be the
 //case
 if(l>v)//if the shower is not seen over the entire track in the atmosphere and l>v. Reset the maximum possible track length to the portion that can be seen
  {
    dMaxDist = dMaxDist>dem-dd ? dem-dd : dMaxDist;
  }
 else//do the same if the shower emerges l<v from telescope v is where the tangent touches earth.
  {
    dMaxDist = dST/sin(elevation) + dInFoV;
    dMaxDist = dMaxDist>dem-dd ? dem-dd : dMaxDist;
  }

 // Double_t delta = acos(-dNy);
 // cout<<"Delta: "<<delta*180/pi<<" angle from tip of shower to telescope: "<<asin(y*sin(delta)/sqrt(y*y+dMaxDist*dMaxDist-2*dMaxDist*y*cos(delta)))*180/pi  <<" shortest distance so Cherenkov light goes to camera: "<<dd<<endl;


  Double_t dTermInSquareRoot = cos(elevation)*cos(azimuth)*cos(elevation)*cos(azimuth)
                               + sin(elevation)*sin(elevation)/tanFoV/tanFoV
                               - cos(elevation)*cos(elevation);

  if(dTermInSquareRoot>0) //if it is negative the shower is contained in the FoV anywhere along the track
    {
        //maximum trajectory length before the tip of the shower is not contained in the camera anymore
       Double_t dMaxDisttoSatisfyFovReq = l / ( cos(elevation)*cos(azimuth) + sqrt( dTermInSquareRoot )) + dInFoV;

       if(dMaxDist>dMaxDisttoSatisfyFovReq+dInFoV && dMaxDisttoSatisfyFovReq>0) //correct max. trajectory length
           dMaxDist = dMaxDisttoSatisfyFovReq+dInFoV;
   //    cout<<" dMaxDisttoSatisfyFovReq: "<<dMaxDisttoSatisfyFovReq<<endl;
    }

 
 
  
  //ok have taken all requirements into account. Lets see if the trajectory
  //length allows for a full shower development.
  //If not we quit.
  if(dMaxDist<dShwrLgth)
   return 0;
  //make sure the shower does not develop past the point where more than 90% of the
  //taus have decayed
  Double_t DecayLength = Etau * c * DecayTime / Mtau;
  Double_t d90PctDecayLength = -log(0.1)*DecayLength;
  //cout<<"90% of taus decayed after: "<<d90PctDecayLength<<endl;

  if(d90PctDecayLength+dShwrLgth<dMaxDist)
    dMaxDist = d90PctDecayLength+dShwrLgth;

  //now lets check if the size of the shower is fullfilling the minimum length
  //requirement

  //calculating length of shower in the camera assuming the shower happens late
  //and develops up to the maximumg possible point along the trajectory
  //add that distance is within maximum distance like we do for the Fluorescence
  //part
  Double_t n = dMaxDist - dShwrLgth -dInFoV;
  Double_t m = dMaxDist;

  Double_t A = sqrt( n* dNx * n* dNx + (n*dNy+y) * (n*dNy+y) + n*dNz * n*dNz  );
  Double_t B = sqrt( m* dNx * m* dNx + (m*dNy+y) * (m*dNy+y) + m*dNz * m*dNz  );

  Double_t costheta = n*dNx * m*dNx + (n*dNy+y)*(m*dNy+y) + m*dNz * n*dNz;
  costheta = costheta / (A*B);
  Double_t dLength = acos(costheta)*180/pi;
  //cout<<"size of shower in degrees: "<<dLength<<" cos of angle:  "<<costheta<<endl;


  if(dLength<dMinLength) //shower image is too short
   return 0;


  //Ok finally we are there. Lets decay the tau in the remaining distance we
  //have
 
  //Probabilty that tau survives if it is not in the field of view
  //below works for tau emerging beyond horizon and before horizon 
  Double_t ProbTauDecay = exp(-(dED-dInFoV)/DecayLength); //Tau has to to survive before it becomes visible to the detector

  
  ProbTauDecay *= 1-exp(-(dMaxDist-dShwrLgth)/DecayLength); // then it has to decay before it is out of the FoV

  ProbTauDecay*=0.8;//only 80% of taus make a shower


//if(azimuth*180/pi>90 && azimuth*180/pi<8.1)
//cout<<"Etau: "<<Etau<<" el: "<<elevation*180/pi<<" az: "<<azimuth*180/pi<<" alpha "<<alpha<<" beta "<<180/pi*(pi - asin(sinbeta))<<" Prob: "<<ProbTauDecay<<" l "<<l<<" v "<<v<<" below horizon: "<<dBH<<" MaxDistOfTrack: "<<dMaxDist<<" minimal distance between trajectory and telescope "<<d<<endl;
return ProbTauDecay;
}


void PlotEmergenceProbability()
{

  TH1D *hTau = new TH1D("hTauS","",70,4,11);
  //hTau->SetMaximum(1);
  hTau->GetXaxis()->SetTitle("energy [GeV]");
  hTau->GetYaxis()->SetTitle("F_tau/F_nu");
  TAxis *axis = hTau->GetXaxis();
  int bins = axis->GetNbins();
  Axis_t from = axis->GetXmin();
  Axis_t to = axis->GetXmax();
  Axis_t width = (to - from) / bins;
  Axis_t *new_bins = new Axis_t[bins + 1];
  for (int i = 0; i <= bins; i++) {
     new_bins[i] = TMath::Power(10, from + i * width);
  }
  axis->Set(bins, new_bins);
  TMultiGraph *mg = new TMultiGraph();
  TLegend *leg = new TLegend(0.7,0.4,0.89,0.88,"neutrino energy");


  double Enulog = 11;
  double Enuminlog = 5.9;
  double Enusteplog = 0.5;
  int s=0;
  while(Enulog>Enuminlog)
    {
      //cout<<Enulog<<endl;
      double Enu = pow(10,Enulog);
      TGraph *grProb = new TGraph(); 
      grProb->SetMarkerStyle(20+s);
      TString title;
      title.Form("%0.0e GeV",Enu);
      leg->AddEntry(grProb,title.Data(),"p"); 
      mg->Add(grProb,"lp");
      s++;

      // loop over target thickness
      double d = 0; //in 10^dmin km
      double dmax = 4;
      double dstep = 0.2;
      int p=0;
      while(d<dmax)
       {          
        double targetthickness = pow(10,d); 
        double dSumProb = 0;
        hTau->Reset();
        for(int i=1;i<hTau->GetNbinsX();i++)
           {

               Double_t Etau = hTau->GetBinCenter(i+1); 
               if(hTau->GetBinLowEdge(i+2)<=Enu)
                 {
                    Double_t P = PEtau(targetthickness,Etau,Enu,hTau);
                    //cout<<i+1<<" "<<targetthickness<<" . "<<Etau<<"  "<<Enu<<" P "<<P<<endl;
                  P *= (hTau->GetBinLowEdge(i+1)-hTau->GetBinLowEdge(i));

                    hTau->Fill(Etau,P); 
                    hTau->SetBinError(i,0);
                    dSumProb+=P;
                 }

            }//got the energy spectrum of the taus for this azimuth and elevation

         grProb->SetPoint(p,targetthickness,dSumProb);
         p++;
         d+=dstep;            
        }    
      Enulog-=Enusteplog;
     }
  TCanvas *cProbOfEmergence = new TCanvas("cProbOfEmergence","Probability of emergence",750,500);
  cProbOfEmergence->Draw();
  cProbOfEmergence->SetLogx();
  cProbOfEmergence->SetLogy();
  mg->Draw("a");
  mg->GetXaxis()->SetTitle("target thickness [km]");
  mg->GetXaxis()->SetTitleSize(0.045);
  mg->GetXaxis()->SetTitleOffset(1.1);
  mg->GetXaxis()->SetLabelSize(0.045);
  mg->GetYaxis()->SetTitle("probability of #tau emergence");
  mg->GetYaxis()->SetTitleOffset(1.0);
  mg->GetYaxis()->SetTitleSize(0.045);
  mg->GetYaxis()->SetLabelSize(0.045);
  mg->GetYaxis()->SetRangeUser(1e-6,1);
 // mg->GetXaxis()->SetRangeUser(1,5e3);
  leg->Draw();
  //TF1 *fdeg=new TF1("fdeg","90-180/3.1415*TMath::ASin(0.5*x/6371)",0,1e4);
  cout<<mg->GetXaxis()->GetXmin()<<endl;
  cout<<180/3.1415*TMath::ASin(0.5*mg->GetXaxis()->GetXmin()/6371)<<"  "<<180/3.1415*TMath::ASin(0.5*mg->GetXaxis()->GetXmax()/6371)<<endl;
  TF1 *fdeg=new TF1("fdeg","x",180/3.1415*TMath::ASin(0.5*mg->GetXaxis()->GetXmin()/6371),180/3.1415*TMath::ASin(0.5*mg->GetXaxis()->GetXmax()/6371));
  fdeg->Eval(0);
  TGaxis *degaxis = new TGaxis(mg->GetXaxis()->GetXmin(),1,mg->GetXaxis()->GetXmax(),1,"fdeg",510,"-G");
  degaxis->SetTitle("elevation angle [degrees]");
  degaxis->SetTitleFont(42);
  degaxis->SetLabelFont(42);
  degaxis->Draw();
}


void GetTauDistribution(TH1D *hTauSpec, Double_t d, Double_t Enumin = 1e9, Double_t Enumax = 3.16e9)
{
   hTauSpec->Reset(); //get the energy spectrum of taus coming out of the Earth, starting with nus in the range expmin expMax
   //int nEnuSteps = 20;
   //Double_t DeltaEnu = (Enumax - Enumin)/nEnuSteps;
   Double_t DeltaEnu = 0.1; //logscale
   int nEnuSteps = (0.0001+log10(Enumax) - log10(Enumin))/DeltaEnu; //the 0.0001 is due to small uncertainties making sure we get the right number of steps
   
   Double_t Normalization = (nuIndex-1)/(pow(Enumin,1-nuIndex)-pow(Enumax,1-nuIndex)); 
   if (!multNorm)
	Normalization = 1.0;
   vector<double> Enu;
   vector<double> EnuWeight;
   for(int i=0;i<nEnuSteps;i++)
      {
        //Enu.push_back(Enumin+i*DeltaEnu+0.5*DeltaEnu);
        //NO Enu.push_back(pow(10,(log10(Enumin+i*DeltaEnu)+log10(Enumin+(i+1)*DeltaEnu))*0.5));
        Enu.push_back(pow(10,log10(Enumin)+i*DeltaEnu+DeltaEnu*0.5));
        //EnuWeight.push_back(pow(Enumin+i*DeltaEnu+0.5*DeltaEnu,-nuIndex)*Normalization*DeltaEnu);
        //NO EnuWeight.push_back(pow(Enu[Enu.size()-1],-nuIndex)*Normalization*DeltaEnu);
        EnuWeight.push_back(pow(Enu[Enu.size()-1],-nuIndex)*Normalization*
                       (pow(10,log10(Enumin)+(i+1)*DeltaEnu)-pow(10,log10(Enumin)+i*DeltaEnu)));
      }

   for(int i=1;i<=hTauSpec->GetNbinsX();i++)
       {

          Double_t Etau = hTauSpec->GetBinCenter(i); 

         //loop over all Enu in this energy bin to calculate the sensitivity
          if(bMonoNu) //if we want to simulate only monoenergetic neutrinos (only good for acceptance calculations
            {
               //NO removed because that is transferred into PEtau 
               //if(Etau<=0.8*Enu) //0.8 because I need to account for part of the energy being transferred to the also produced neutrinos
                 {
                    Double_t Enu = pow(10,(log10(Enumin)+log10(Enumax))*0.5);
                    Double_t P = PEtau(d,Etau,Enu,hTauSpec);
                    hTauSpec->Fill(Etau,P);
                 } 
            }
          else
            {
              //while(Enu<Enumax)//loop over nu energy bin
              for(int n = 0; n<nEnuSteps;n++)
                {
                  //NO removed because that is transferred into PEtau 
                  //if(Etau<=0.8*Enu) //0.8 because I need to account for part of the energy being transferred to the also produced neutrinos
                    {
                      Double_t P = 0;
                      //if(Enu==Enumin)  //mod
                      P = PEtau(d,Etau,Enu[n],hTauSpec);
                      //if(Enu==Enumin) //mod
                      //cout<<P<<"  "<<d<<"  "<<Etau<<" "<<Enu[n]<<"  "<<EnuWeight[n]<<endl; //mod
                      //P *= DeltaEnu/(Enumax-Enumin); 
                      P *= EnuWeight[n];

                      //do not know how the below is calculated
                      //P *= ( pow(Enu,1-nuIndex) - pow(Enu+DeltaEnu,1-nuIndex) ) 
                      // / ( pow(Enumin,1-nuIndex) - pow(Enumax,1-nuIndex) );//multiplying in the with
                      hTauSpec->SetBinContent(i,hTauSpec->GetBinContent(i)+P); 
                      //hTauSpec->Fill(Etau,P); 
                    }
                //Enu += DeltaEnu;  
               }
           }    
      }//got the energy spectrum of the taus for this azimuth and elevation

 /*
               for(int i=0;i<hTauSpec->GetNbinsX();i++)
                  {
cout<<i+1<<"  "<<hTauSpec->GetBinCenter(i+1)  <<" taus cont: "<<hTauSpec->GetBinContent(i+1)<<endl;
}
*/
      //finding lowest and highest bin that is non-zero
      //bin with maximum content
      Int_t iBinMax = hTauSpec->GetMaximumBin();
      Double_t dContInMax = hTauSpec->GetBinContent(iBinMax);
      //   cout<<"iBinMax: "<<iBinMax<<" maxcontent: "<<dContInMax<<endl;       
   
      //Double_t dHighE = hTauSpec->GetBinLowEdge(iBinMax+1);
      //finding lowest bin
      Int_t iBinMin = iBinMax;
      while(hTauSpec->GetBinContent(iBinMin)>1e-4*dContInMax)
        {
           iBinMin--;   
        }
      // cout<<"iBinMin: "<<iBinMin<<endl;

     //finding highest bin
      while(hTauSpec->GetBinContent(iBinMax+1)>0)
         iBinMax++;
 
      //cout<<"iBinMax: "<<iBinMax<<endl;       


       //Double_t dLowE = hTauSpec->GetBinLowEdge(iBinMin);
       //Double_t dDeltaE = dHighE-dLowE;

       for(int i=1;i<hTauSpec->GetNbinsX();i++) //weight each bin with energy scale need to do that because of logarithmic energy scale
          {
            Double_t dContent = hTauSpec->GetBinContent(i);
            dContent *= (hTauSpec->GetBinLowEdge(i+1)-hTauSpec->GetBinLowEdge(i));
            if(i<iBinMin || i>iBinMax)
                 dContent = 0;
            hTauSpec->SetBinContent(i,dContent);
            hTauSpec->SetBinError(i,0);
          }

}


//Calculates the acceptance by looping over distance, azimuth and elevation for
//a given energy bin
//returns the integral acceptance
//and returns a graph that holds the acceptance for each step in distance in the
//loop
Double_t CalculateAcceptance(Double_t dMinEnu, Double_t dMaxEnu,TGraph *grDiffAcceptance,TH1D *hTau)
{
   dMinEnu = pow(10,dMinEnu);
   dMaxEnu = pow(10,dMaxEnu);

   //the hTau passed to the function is ignored. Create a new one below
   //hTau = new TH1D("hTauNew","",dMaxEnu/(dMinEnu/10.),dMinEnu/10.,dMaxEnu);
/*
//make logarithmic axis
TAxis *axis = hTauNew->GetXaxis();
int bins = axis->GetNbins();
Axis_t from = axis->GetXmin();
Axis_t to = axis->GetXmax();
Axis_t width = (to - from) / bins;
Axis_t *new_bins = new Axis_t[bins + 1];
for (int i = 0; i <= bins; i++) {
     new_bins[i] = TMath::Power(10, from + i * width);
}
axis->Set(bins, new_bins);
axis->Delete();
*/
    //area of cell
    Double_t dConversion=yDelta*2*pi; //multiply area of cell taking into account that we have a 360 degree FoV
    dConversion*=1e10; //from km2 to cm2
    //solid angle
    dConversion*=DeltaAngleAz/180.*pi*DeltaAngle/180.*pi; //multiply area of solidangle cell

    //time Do that in the sensitivity calculation. Acceptance is calculated
    //without the observing time
    //dConversion*=3*365*24*3600*0.20; //exposure time 3 years in seconds with 20% duty cycle

   
    dConversion*=2; //because we only calculate for azimuth angles 0 to azimuth max. There are also negative azimuth values due to symmetry of the problem 


    Double_t dIntegratedAcceptance=0;
    Int_t p = 0;
    Double_t y = yMin; //y distance from telescope where tau comes out of the ground;
    while(y<yMax) //loop over distance to telescope
     {
       //cout<<"Distance from Detector: "<<y<<endl;
       //calculate for given elevation the length of the trajectory through earth.
       Double_t dAcceptance=0.0;

       Double_t MaxAzimuth = dMaxCherenkovAzimuthAngle;
       if( bFluorescence || (bCombined && y<dMaxFluorescenceDistance) ) // so we can make full use of fluoresence events
         MaxAzimuth = 180;

       Double_t elevation=DeltaAngle*0.5;    
       while(elevation<MaxElevation) //loop over elevation
         {
            Double_t dWeightForTriggeredAzimuth = sin(elevation/180.*pi)*y;
            Double_t azimuth = DeltaAngleAz*0.5;
            while(azimuth<MaxAzimuth) // loop over azimuth
              {

                 if(azimuth>MaxAzimuth && y>dMaxFluorescenceDistance)
                  cout<<" Azimuth:" <<azimuth<<" should not be here "<<endl;

                 Double_t dEarth = DistanceThroughEarth(y,elevation,azimuth);
                 //cout<<"   length of trajectory in Earth: "<<dEarth<<" km"<<endl;

                 //cout<<dEarth<<"  "<<dMinEnu<<"  "<<dMaxEnu<<endl; 
                 GetTauDistribution(hTau,dEarth,dMinEnu,dMaxEnu);                
                 //cout<<"done GetTau"<<endl;

               Double_t dDeltaAcceptance=0;

               //Calculate probability that taus with E convert before they are 150 km away from detector when they make it out of the earth. 
               //150km is for an angle of 5 degrees assuming 10 degree opening angle. If the angle is free the distance of closest approach depends on
               // where the tau comes out of the earth and under what angle (vertical and horizontal. 
               //That is probably dependend on the initial nu energy
               Double_t dP = 0;
                //     if(elevation>0.5 && elevation<1 && azimuth>9 && azimuth<9.2)
               for(int i=0;i<hTau->GetNbinsX();i++)
                  {
//cout<<dMinEnu<<"  "<<dMaxEnu<<endl;
//cout<<"taus cont: "<<hTau->GetBinContent(i+1)<<endl;
                     if(hTau->GetBinContent(i+1)>0)
                      {
                        Double_t dPFluorescence = 0.0;
                        Double_t dPCherenkov = 0.0;
                         //cout<<bFluorescence<<" "<<bCombined<<"  "<<y<<"<"<<dMaxFluorescenceDistance<<endl;
                        if( bFluorescence || (bCombined && y<dMaxFluorescenceDistance) )
                           dPFluorescence = PDecayFluorescence(hTau->GetBinCenter(i+1),y,elevation,azimuth);
                        if( (!bFluorescence || bCombined) && azimuth<dMaxCherenkovAzimuthAngle  )
                            dPCherenkov = PDecay(hTau->GetBinCenter(i+1),y,elevation,azimuth);

                        if(bCombined)
                         dP = dPFluorescence > dPCherenkov ? dPFluorescence : dPCherenkov;
                        else if(bFluorescence)
                          dP = dPFluorescence;
                        else
                          dP = dPCherenkov;
                        dDeltaAcceptance+=hTau->GetBinContent(i+1)*dP;
                        //dDeltaAcceptance+=hTau->GetBinContent(i+1); //use above
                        if(!bMonoNu)
                          hTriggeredAzimuthAngles->Fill(azimuth,hTau->GetBinContent(i+1)*dP*dWeightForTriggeredAzimuth);
                       }
                     //if(hTau->GetBinContent(i+1)*dP>0 )
                     //cout<<hTau->GetBinCenter(i+1)<<"  "<<hTau->GetBinContent(i+1)<<" y:  "<<y<<"  el: "<<elevation<<" az: "<<azimuth<<" dp: "<<dP<<" dDeltaAccept: "<<dDeltaAcceptance<<" prod: "<<hTau->GetBinContent(i+1)*dP<<endl;
                  }
               if(dDeltaAcceptance<1e-10 && y>50) //won't get any more acceptance. The >60 is to make sure we do not miss fluorescence events whic can be seen from the back
                   break;
               dAcceptance+=dDeltaAcceptance*sin(elevation/180.*pi); //projection of area cell to trajectory
               //   cout<<"distance "<<y<<" prob"<<dDeltaAcceptance<<" elevation  "<<elevation<<endl;
               azimuth+=DeltaAngleAz;

               //Add absorption in the atmosphere between shower and observer
               //Go over target area and calculate acceptance angle for each dA. Integrate over energy spectrum of taus coming out of the earth at that point. multiplied with detection efficiency(absorption).
             }//finished looping over all azimuth angles
               elevation+=DeltaAngle;    
             //cout<<azimuth<<"  "<<dAcceptance<<endl;
           //multiply with dOmega  DeltaAngle*DeltaAngle
        }//finished looping over all elevation angles
    //dAcceptance*=yDelta*DeltaAngle/180*pi*y; //multiply area of cell
    //dAcceptance*=DeltaAngle/180*pi*DeltaAngle/180*pi; //multiply area of solidangle cell
    dAcceptance*=y; //multiply with area of cell (note that the yDelta*DeltaAngle/180*pi is included in dConversion
    dIntegratedAcceptance+=dAcceptance;
    
    grDiffAcceptance->SetPoint(p,y,dAcceptance*dConversion); 
    p++;
    cout<<"distance: "<<y<<" differ. acceptance "<<(dAcceptance*dConversion)<<" integr. acceptance.: "<<(dIntegratedAcceptance*dConversion)<<endl; 
    cout<<dAcceptance<<"  "<<dConversion<<endl;
    y+=yDelta;
    if(dAcceptance<1e-10 && y>50) //no sense to increase in distance if we can't see any showers now
       break;
       //~ break;
  }//end looping over distances
  //~ cout<<"adaasdas "<<dIntegratedAcceptance<<endl;
return (dIntegratedAcceptance*dConversion);
}

/*
Double_t CalculateAcceptance2(Double_t dMinEnu, Double_t dMaxEnu,TGraph *grDiffAcceptance,TH1D *hTau)
{

	dMinEnu = pow(10,dMinEnu);
   dMaxEnu = pow(10,dMaxEnu);
   Double_t latitude = 38.52028; //lat of frisco peak, utah
	Double_t LST = 212.33; // Local Sidereal Time 15:09:19 corresponds to 00:00:00 UTC on 9/01/2020 (start date and time)
	Double_t degconv = pi/180.0;
	Double_t tStep = 2.5; //10 min step in degrees
	Double_t hour = 15.0; //1 hour in degrees
	Double_t day = 360.0; //day in degrees
	
    TCanvas *skyC = new TCanvas("skyC","Skymap of Acceptance",1600,750); //new canvas for skymap
    TGraph  *plot = new TGraph(); //for plotting galactic landmarks
    skyC->Divide(2,1);
    //~ TH2F *skymap = new TH2F("skymap","Acceptance Skymap", 180, -180, 180, 179, -89.5, 89.5);
    skymapSingleAngle->GetXaxis()->SetTitle("Azimuth Angle [degrees]");
    skymapSingleAngle->GetYaxis()->SetTitle("Elevation Angle [degrees]");
    skymapFull360Sweep->GetXaxis()->SetTitle("Azimuth Angle [degrees]");
    skymapFull360Sweep->GetYaxis()->SetTitle("Elevation Angle [degrees]");
    skymapSingleAngle->SetStats(1);
    skymapFull360Sweep->SetStats(1);
    //area of cell
    Double_t dDeltaTelescopeAzimuth=DeltaAngleAz;
    Double_t dConversion=yDelta*dDeltaTelescopeAzimuth*pi/180.0; //multiply area of cell taking into account that we have a 360 degree FoV
    dConversion*=1e10; //from km2 to cm2
    //solid angle
    //~ dConversion*=DeltaAngleAz/180.*pi*DeltaAngle/180.*pi; //multiply area of solidangle cell

    //time Do that in the sensitivity calculation. Acceptance is calculated
    //without the observing time
    //dConversion*=3*365*24*3600*0.20; //exposure time 3 years in seconds with 20% duty cycle

   
    //~ dConversion*=2; //because we only calculate for azimuth angles 0 to azimuth max. There are also negative azimuth values due to symmetry of the problem 
	//~ int nbinsx = 0;
	//~ int nbinsy = 0;

    Double_t dIntegratedAcceptance=0;
    Int_t p = 0;
    //~ Double_t y = yMin ; //y distance from telescope where tau comes out of the ground;
    Double_t y = 20; //y distance from telescope where tau comes out of the ground;
    //~ cout<<"y before the loop is: "<<y<<" yMax is: "<<yMax<<" yMin is: "<<yMin<<" yDelta is: "<<yDelta<<endl;
    while(y<yMax) //loop over distance to telescope
     {
       //cout<<"Distance from Detector: "<<y<<endl;
       //calculate for given elevation the length of the trajectory through earth.
       Double_t dAcceptance=0.0;

       //~ Double_t MaxAzimuth = dMaxCherenkovAzimuthAngle;
       Double_t MaxAzimuth = 180.0;
       if( bFluorescence || (bCombined && y<dMaxFluorescenceDistance) ) // so we can make full use of fluoresence events
         MaxAzimuth = 180.0;
		MaxElevation = 40.0;
       //~ Double_t elevation=DeltaAngle*0.5;    
       Double_t elevation=0;    
       //~ while(elevation<MaxElevation) //loop over elevation
       for(int elv = 0; elv <= (int)(MaxElevation * 10); elv++)
         {
			 elevation = elv/10.0;
            Double_t dWeightForTriggeredAzimuth = sin(elevation/180.*pi)*y;
            Double_t azimuth = 0;
            //~ while(azimuth<MaxAzimuth) // loop over azimuth
            for(int azi = 0; azi <= (int)(MaxAzimuth * 10); azi++)
              {
				  azimuth = azi/10.0;
				  //~ cout<<"azi: "<<azimuth<<endl;

                 if(azimuth>MaxAzimuth && y>dMaxFluorescenceDistance)
                  cout<<" Azimuth:" <<azimuth<<" should not be here "<<endl;

                 Double_t dEarth = DistanceThroughEarth(y,elevation,azimuth);
                 //cout<<"   length of trajectory in Earth: "<<dEarth<<" km"<<endl;

                 //cout<<dEarth<<"  "<<dMinEnu<<"  "<<dMaxEnu<<endl; 
                 GetTauDistribution(hTau,dEarth,dMinEnu,dMaxEnu);                

               Double_t dDeltaAcceptance=0;

               //Calculate probability that taus with E convert before they are 150 km away from detector when they make it out of the earth. 
               //150km is for an angle of 5 degrees assuming 10 degree opening angle. If the angle is free the distance of closest approach depends on
               // where the tau comes out of the earth and under what angle (vertical and horizontal. 
               //That is probably dependend on the initial nu energy
               Double_t dP = 0;
                //     if(elevation>0.5 && elevation<1 && azimuth>9 && azimuth<9.2)
               for(int i=0;i<hTau->GetNbinsX();i++)
                  {
//cout<<dMinEnu<<"  "<<dMaxEnu<<endl;
//cout<<"taus cont: "<<hTau->GetBinContent(i+1)<<endl;
                     if(hTau->GetBinContent(i+1)>0)
                      {
                        Double_t dPFluorescence = 0.0;
                        Double_t dPCherenkov = 0.0;
                         //cout<<bFluorescence<<" "<<bCombined<<"  "<<y<<"<"<<dMaxFluorescenceDistance<<endl;
                        if( bFluorescence || (bCombined && y<dMaxFluorescenceDistance) )
                           dPFluorescence = PDecayFluorescence(hTau->GetBinCenter(i+1),y,elevation,azimuth);
                        if( (!bFluorescence || bCombined) && azimuth<dMaxCherenkovAzimuthAngle  )
                            dPCherenkov = PDecay(hTau->GetBinCenter(i+1),y,elevation,azimuth);

                        if(bCombined)
                         dP = dPFluorescence > dPCherenkov ? dPFluorescence : dPCherenkov;
                        else if(bFluorescence)
                          dP = dPFluorescence;
                        else
                          dP = dPCherenkov;
                        dDeltaAcceptance+=hTau->GetBinContent(i+1)*dP;
                        //dDeltaAcceptance+=hTau->GetBinContent(i+1); //use above
                        hTriggeredAzimuthAngles->Fill(azimuth,hTau->GetBinContent(i+1)*dP*dWeightForTriggeredAzimuth);
                       }
                     //if(hTau->GetBinContent(i+1)*dP>0 )
                     //cout<<hTau->GetBinCenter(i+1)<<"  "<<hTau->GetBinContent(i+1)<<" y:  "<<y<<"  el: "<<elevation<<" az: "<<azimuth<<" dp: "<<dP<<" dDeltaAccept: "<<dDeltaAcceptance<<" prod: "<<hTau->GetBinContent(i+1)*dP<<endl;
                  }
                  					
                  if(azimuth != 0.0) {
					//~ if(dDeltaAcceptance == 0.0)
						//~ skymapSingleAngle->Fill(azimuth, (-1 * elevation), -0.0001);
					//~ else
						skymapSingleAngle->Fill(azimuth, (-1 * elevation), dDeltaAcceptance*sin(elevation/180.*pi)*y*dConversion);
					//~ skymapFull360Sweep->Fill(azimuth, (-1 * elevation), dDeltaAcceptance*sin(elevation/180.*pi)*y*dConversion);
				}
				//~ if(dDeltaAcceptance == 0.0)
					//~ skymapSingleAngle->Fill((-1 * azimuth), (-1 * elevation), -0.0001);
				//~ else
					skymapSingleAngle->Fill((-1 * azimuth), (-1 * elevation), dDeltaAcceptance*sin(elevation/180.*pi)*y*dConversion);
				//~ skymapFull360Sweep->Fill((-1 * azimuth), (-1 * elevation), dDeltaAcceptance*sin(elevation/180.*pi)*y*dConversion);
				
                  //~ cout<<"Azimuth: "<<azimuth<<" degrees, Elevation: "<<elevation<<" degrees, Acceptance: "<<dDeltaAcceptance*sin(elevation/180.*pi)*y*dConversion<<endl;
				
                  
               if(dDeltaAcceptance<1e-20 && y>50) //won't get any more acceptance. The >60 is to make sure we do not miss fluorescence events whic can be seen from the back
                   break;

               //~ dAcceptance+=dDeltaAcceptance*sin(elevation/180.*pi); //projection of area cell to trajectory
               //~ skymapFull360Sweep->Fill(azimuth, (-1)* elevation, dAcceptance*y*dConversion);
               //~ skymapFull360Sweep->Fill((-1) * azimuth, (-1)* elevation, dAcceptance*y*dConversion);
               //   cout<<"distance "<<y<<" prob"<<dDeltaAcceptance<<" elevation  "<<elevation<<endl;
               // Skymap stuff goes here (before the azimuth gets updated).
				
               //~ cout<<"right ascent: "<<ra<<" decl: "<<dec<<endl;
               //~ skymap->Draw("aitoff");
               //~ skyC->Modified();
               //~ skyC->Update();
               //~ cout<<" Azimuth:" <<azimuth<<endl;
                 //~ cout<<"Azi: "<<azimuth<<", Elv: "<<elevation<<endl;
               //~ azimuth+=DeltaAngleAz;
				//~ nbinsx++;
               //Add absorption in the atmosphere between shower and observer
               //Go over target area and calculate acceptance angle for each dA. Integrate over energy spectrum of taus coming out of the earth at that point. multiplied with detection efficiency(absorption).
             }//finished looping over all azimuth angles
             
               //~ elevation+=DeltaAngle;    
               //~ nbinsy++;
             //cout<<azimuth<<"  "<<dAcceptance<<endl;
           //multiply with dOmega  DeltaAngle*DeltaAngle
           //~ cout<<azimuth<<" azi"<<endl;
        }//finished looping over all elevation angles
    //dAcceptance*=yDelta*DeltaAngle/180*pi*y; //multiply area of cell
    //dAcceptance*=DeltaAngle/180*pi*DeltaAngle/180*pi; //multiply area of solidangle cell
    dAcceptance*=y; //multiply with area of cell (note that the yDelta*DeltaAngle/180*pi is included in dConversion
    dIntegratedAcceptance+=dAcceptance;
    
    grDiffAcceptance->SetPoint(p,y,dAcceptance*dConversion); 
    p++;
    cout<<"distance: "<<y<<" differ. acceptance "<<(dAcceptance*dConversion)<<" integr. acceptance.: "<<(dIntegratedAcceptance*dConversion)<<endl; 
    cout<<dAcceptance<<"  "<<dConversion<<endl;
    y+=yDelta;
    if(dAcceptance<1e-10 && y>50) //no sense to increase in distance if we can't see any showers now
       break;
       break;
  }//end looping over distances
//~ for(int xBinWidth = skymapSingleAngle->GetNbinsX(); xBinWidth > 0; xBinWidth--){
	//~ for(int xbin = 1; xbin <= skymapSingleAngle->GetNbinsX() - xBinWidth + 1; xbin++){
		//~ for(int ybin = 1; ybin <= skymapSingleAngle->GetNbinsY(); ybin++){
			//~ Double_t comboBin = skymapFull360Sweep->GetBinContent(xbin, ybin);
			//~ comboBin += skymapSingleAngle->GetBinContent((xBinWidth + xbin) - 1, ybin);
			//~ skymapFull360Sweep->SetBinContent(xbin, ybin, comboBin);
		//~ }
	//~ }
	//~ cout<<xBinWidth<<endl;
//~ }

//~ for(int xBinWidth = 1; xBinWidth < skymapSingleAngle->GetNbinsX(); xBinWidth++){
	//~ for(int xbin = skymapSingleAngle->GetNbinsX(); xbin >= skymapSingleAngle->GetNbinsX() - xBinWidth + 1; xbin--){
		//~ for(int ybin = 1; ybin <= skymapSingleAngle->GetNbinsY(); ybin++){
			//~ Double_t comboBin = skymapFull360Sweep->GetBinContent(xbin, ybin);
			//~ comboBin += skymapSingleAngle->GetBinContent(xbin - skymapSingleAngle->GetNbinsX() + xBinWidth, ybin);
			//~ skymapFull360Sweep->SetBinContent(xbin, ybin, comboBin);
		//~ }
	//~ }
	//~ cout<<xBinWidth<<endl;
//~ }

for(int yBins = 1; yBins <= skymapSingleAngle->GetNbinsY(); yBins++){
	Double_t comboBin = 0;
	for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
		comboBin += skymapSingleAngle->GetBinContent(xBins, yBins);
	for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
		skymapFull360Sweep->SetBinContent(xBins, yBins, comboBin);
	//~ cout<<yBins<<endl;
}

  //~ skymapFull360Sweep->Add(skymapSingleAngle);
//~ skymapSingleAngle->Fill(0.0,0.0, -0.1);
  //~ skymapFull360Sweep->Fill(0.0,10.1, -10);
  //~ cout<<"orgin: "<<skymapSingleAngle->GetBinContent(0.0, 0.0)<<endl;
  //~ cout<<"peak: "<<skymapSingleAngle->GetBinContent(65.0, -1.0)<<endl;
  //~ cout<<"nx: "<<nbinsx<<" ny: "<<nbinsy<<endl;
  skyC->cd(1);
  gPad->SetLogz(1);
  skymapSingleAngle->GetXaxis()->SetTitle("Azimuth [deg]");
  skymapSingleAngle->GetYaxis()->SetTitle("Elevation [deg]");
skymapSingleAngle->Draw("COLZ");
skyC->cd(2);
gPad->SetLogz(1);
skymapFull360Sweep->GetXaxis()->SetTitle("Azimuth [deg]");
skymapFull360Sweep->GetYaxis()->SetTitle("Elevation [deg]");
skymapFull360Sweep->Draw("COLZ");

//projection onto the sky

//~ for(int xBins = 1; xBins <= skymapFull360Sweep->GetNbinsX(); xBins++) {
	//~ Double_t az = xBins * 0.1 - 180.1;
	//~ for(int yBins = 1; yBins <= skymapFull360Sweep->GetNbinsY(); yBins++) {
		//~ Double_t alt = yBins * 0.1 - 90.1;
		//~ Double_t dec = asin(sin(alt * degconv) * sin(latitude * degconv) + cos(alt * degconv) * cos(latitude * degconv) * cos(az * degconv)) * (180.0 / pi);
		//~ Double_t ra = atan2(sin((az + 180.0) * degconv), (cos((az + 180.0) * degconv) * sin(latitude * degconv) + tan(alt * degconv) * cos(latitude * degconv))) * (180.0 / pi);
		//~ ra = LST - ra;
		//~ if(ra > 180.0)
			//~ ra = ra - 360.0;
		//~ skymapFullProjection->Fill(ra, dec, skymapFull360Sweep->GetBinContent(xBins, yBins));
	//~ }
//~ }

//~ for(int t = 1; t <= (int)(180.0 / tStep); t++) {
	//~ for(int r = -180 * 10; r <= 180 * 10; r++) {
		//~ Double_t ra = r / 10.0;
		//~ for(int d = -90 * 10; d <= 90 * 10; d++) {
			//~ Double_t dec = d / 10.0;
			//~ Double_t az = (atan2(sin((LST - ra) * degconv), cos((LST - ra) * degconv) * sin(latitude * degconv) - tan(dec * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
			//~ Double_t alt = asin(sin(latitude * degconv) * sin(dec * degconv) + cos(latitude * degconv) * cos(dec * degconv) * cos((LST - ra) * degconv)) * 180 / pi;
			//~ if(az > 180.0)
				//~ az = az - 360.0;
			//~ if(az < -180.0)
				//~ az = az + 360.0;
			//~ int xBin = (int)((az + 180.1) * 10);
			//~ int yBin = (int)((alt + 90.1) * 10);
			//~ skymapFullProjection->Fill(ra, dec, skymapFull360Sweep->GetBinContent(xBin, yBin));
		//~ }
	//~ }
	//~ LST += tStep;
	//~ if(LST > 360.0)
		//~ LST = LST - 360;
//~ }

ifstream in;
in.open("10yrs.txt"); //open xephem file

Double_t totalT, totalAcc;

if (in.is_open()) { //if the xephem file is open
	string sTimeS, rTimeS, rTimeM, sTimeM, phM;
	//~ int tens, ones, hour, min;
	Double_t setTimeSun, riseTimeSun, riseTimeMoon, setTimeMoon, phaseMoon, deltaT = 0, resT, endT;
	bool nestNone = false, nestSun = false, nestBoth = false;
	
	while(in.good()) {
		in >> sTimeS >> rTimeS >> rTimeM >> sTimeM >> phM;
		
		//~ setTimeSun = ((sTimeS[0] - '0') * 10 + (sTimeS[1] - '0')) / 24.0 * day + ((sTimeS[3] - '0') * 10 + (sTimeS[4] - '0')) / 1440.0 * day;
		//~ riseTimeSun = (rTimeS[0] - '0') / 24.0 * day + ((rTimeS[2] - '0') * 10 + (rTimeS[3] - '0')) / 1440.0 * day;
		
		setTimeSun = stod(sTimeS);
		riseTimeSun = stod(rTimeS);
		riseTimeMoon = stod(rTimeM);
		setTimeMoon = stod(sTimeM);
		phaseMoon = stod(phM);
		
		//~ if(setTimeSun > riseTimeSun)
			//~ deltaT = (360.0 - setTimeSun) + riseTimeSun;
		//~ else 
			//~ deltaT = riseTimeSun - setTimeSun;
		
		//~ totalT += deltaT;
		//~ LST = setTimeSun;
		
		nestNone = false;
		nestSun = false;
		nestBoth = false;
		
		if(phaseMoon < 0.3) {
			if(setTimeSun > riseTimeSun)
				deltaT = (360.0 - setTimeSun) + riseTimeSun;
			else
				deltaT = riseTimeSun - setTimeSun;
			LST = setTimeSun;
		} else {
			if( (riseTimeSun > setTimeSun) && (setTimeMoon > riseTimeMoon) ) { //both don't cross 0hr
				if( (riseTimeMoon < setTimeSun) && (setTimeMoon < setTimeSun) && (setTimeMoon < riseTimeSun) ){
					LST = setTimeMoon;
					deltaT = riseTimeSun - setTimeMoon;
					endT = riseTimeSun;
				} else if( (riseTimeMoon > setTimeSun) && (setTimeMoon > riseTimeSun) ) {
					LST = setTimeSun;
					deltaT = riseTimeMoon - setTimeSun;
					endT = riseTimeMoon;
				} else if( (riseTimeMoon > setTimeSun) && (setTimeMoon < riseTimeSun) ) {
					LST = setTimeSun;
					deltaT = (riseTimeMoon - setTimeSun) + (riseTimeSun - setTimeMoon);
					nestNone = true;
					endT = riseTimeSun;
				} else if( (riseTimeMoon < setTimeSun && setTimeMoon < setTimeSun) || (riseTimeMoon > riseTimeSun && setTimeMoon > riseTimeSun) ) {
					LST = setTimeSun;
					deltaT = riseTimeSun - setTimeSun;
					endT = riseTimeSun;
				} else { deltaT = 0; }
			} else if( (riseTimeSun < setTimeSun) && (setTimeMoon > riseTimeMoon) ) { //only sun crosses 0hr (impossible for moon to cover full night)
				if( (riseTimeMoon < setTimeSun) && (setTimeMoon > setTimeSun) ) {
					LST = setTimeMoon;
					deltaT = (360. + riseTimeSun) - setTimeMoon;
					endT = riseTimeSun;
				} else if( (riseTimeSun > riseTimeMoon) && (riseTimeSun < setTimeMoon) ) {
					LST = setTimeSun;
					deltaT = (360. + riseTimeMoon) - setTimeSun;
					endT = riseTimeMoon;
				} else if( (riseTimeMoon > setTimeSun && setTimeMoon < 360.) || (riseTimeMoon < riseTimeSun && setTimeMoon < riseTimeSun) ) {
					LST = setTimeSun;
					deltaT = (riseTimeSun + 360. - setTimeSun) - (setTimeMoon - riseTimeMoon);
					nestSun = true;
					endT = riseTimeSun;
				} else if( (riseTimeMoon < setTimeSun && setTimeMoon < setTimeSun) || (riseTimeMoon > riseTimeSun && setTimeMoon > riseTimeSun) ) {
					LST = setTimeSun;
					deltaT = (360. + riseTimeSun) - setTimeSun;
					endT = riseTimeSun;
				}
			} else if( (riseTimeSun > setTimeSun) && (setTimeMoon < riseTimeMoon) ) { //only moon crosses 0hr
				if( ((setTimeSun + 360.) > riseTimeMoon) && (setTimeMoon > setTimeSun) && (setTimeMoon < riseTimeSun) ) {
					LST = setTimeMoon;
					deltaT = riseTimeSun - setTimeMoon;
					endT = riseTimeSun;
				} else if( (riseTimeMoon < riseTimeSun) && (riseTimeSun < (setTimeMoon + 360.)) && (riseTimeMoon > setTimeSun) ) {
					LST = setTimeSun;
					deltaT = riseTimeMoon - setTimeSun;
					endT = riseTimeMoon;
				} else if( (riseTimeMoon > riseTimeSun) && (setTimeMoon < setTimeSun) ) {
					LST = setTimeSun;
					deltaT = riseTimeSun - setTimeSun;
					endT = riseTimeSun;
				} else { deltaT = 0; }
			} else if ( (riseTimeSun < setTimeSun)  && (setTimeMoon < riseTimeMoon) ) { //both cross 0 hr
				if( (riseTimeMoon > setTimeSun) && (riseTimeSun < setTimeMoon) ) {
					LST = setTimeSun;
					deltaT = riseTimeMoon - setTimeSun;
					endT = riseTimeMoon;
				} else if( (riseTimeMoon < setTimeSun) && (setTimeMoon < riseTimeSun) ) {
					LST = setTimeMoon;
					deltaT = riseTimeSun - setTimeMoon;
					endT = riseTimeSun;
				} else if( (setTimeSun < riseTimeMoon) && (setTimeMoon < riseTimeSun) ) {
					LST = setTimeSun;
					deltaT = (riseTimeSun + 360. - setTimeSun) - (setTimeMoon + 360. - riseTimeMoon);
					nestBoth = true;
					endT = riseTimeSun;
				} else { deltaT = 0; }
			}
		}
		
		totalT += deltaT;
		
		int nSteps = (int)(deltaT / tStep);
		
		Double_t tStepAdjusted = deltaT / nSteps;
		
		
		for(int i = 0; i < nSteps; i++) {
			for(int r = -180; r <= 180; r++) {
				//~ Double_t ra = r;
				for(int d = -90; d <= 90; d++) {
					Double_t dec = asin(sin(d * degconv) * sin(27.1284 * degconv) + cos(d * degconv) * cos(27.1284 * degconv) * cos((122.9320 - r) * degconv)) * 180 / pi;
					Double_t ra = (atan2((cos(d * degconv) * sin((122.932 - r) * degconv)), (sin(d * degconv) * cos(27.1284 * degconv) - cos(d * degconv) * sin(27.1284 * degconv) * cos((122.9320 - r) * degconv))) * 180 / pi) + 192.8595;
					if(ra > 180)
						ra = ra - 360.;
					if(ra < -180)
						ra = ra + 360.;
					Double_t az = (atan2(sin((LST - ra) * degconv), cos((LST - ra) * degconv) * sin(latitude * degconv) - tan(dec * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
					Double_t alt = asin(sin(latitude * degconv) * sin(dec * degconv) + cos(latitude * degconv) * cos(dec * degconv) * cos((LST - ra) * degconv)) * 180 / pi;
					if(az > 180.0)
						az = az - 360.0;
					if(az < -180.0)
						az = az + 360.0;
					int xBin = (int)((az + 180.1) * 10);
					int yBin = (int)((alt + 90.1) * 10);
					if( !(nestNone && LST > riseTimeMoon && LST < setTimeMoon) && 
						!(nestSun && LST > riseTimeMoon && LST < setTimeMoon) && 
						!(nestBoth && ( (LST > riseTimeMoon && LST < 360.) || (LST > 0 && LST < setTimeMoon) )) ) 
						{ skymapFullProjection->Fill((-1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin) * tStepAdjusted * 240); }
						//~ { skymapFullProjection->Fill((1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin)); }
				}
			}
			LST += tStepAdjusted;
			if(LST > 360.0)
				LST -= 360.0;
		}
		
		//~ if(LST > endT)
			//~ resT = endT + 360. - LST;
		//~ else
			//~ resT = endT - LST;
		
		//~ if(resT > (tStep / 2.0) && deltaT != 0.0) {
			//~ LST += tStep;
			//~ if(LST > 360.0)
				//~ LST -= 360.0;
			//~ totalT += tStep;
			//~ for(int r = -180; r <= 180; r++) {
					//~ Double_t ra = r;
					//~ for(int d = -90; d <= 90; d++) {
						//~ Double_t dec = asin(sin(d * degconv) * sin(27.1284 * degconv) + cos(d * degconv) * cos(27.1284 * degconv) * cos((122.9320 - r) * degconv)) * 180 / pi;
						//~ Double_t ra = (atan2((cos(d * degconv) * sin((122.932 - r) * degconv)), (sin(d * degconv) * cos(27.1284 * degconv) - cos(d * degconv) * sin(27.1284 * degconv) * cos((122.9320 - r) * degconv))) * 180 / pi) + 192.8595;
						//~ if(ra > 180)
							//~ ra = ra - 360.;
						//~ if(ra < -180)
							//~ ra = ra + 360.;
						//~ Double_t az = (atan2(sin((LST - ra) * degconv), cos((LST - ra) * degconv) * sin(latitude * degconv) - tan(dec * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
						//~ Double_t alt = asin(sin(latitude * degconv) * sin(dec * degconv) + cos(latitude * degconv) * cos(dec * degconv) * cos((LST - ra) * degconv)) * 180 / pi;
						//~ if(az > 180.0)
							//~ az = az - 360.0;
						//~ if(az < -180.0)
							//~ az = az + 360.0;
						//~ int xBin = (int)((az + 180.1) * 10);
						//~ int yBin = (int)((alt + 90.1) * 10);
						//~ skymapFullProjection->Fill((-1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin) * tStep * 240);
						//~ skymapFullProjection->Fill((1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin));
					//~ }
				//~ }
		//~ break;
		//~ }
	}
} else cout << "Unable to open file"; 

for(int i = 1; i <= skymapFullProjection->GetNbinsX(); i++) {
	for(int j = 1; j <= skymapFullProjection->GetNbinsY(); j++)
		totalAcc += skymapFullProjection->GetBinContent(i, j);
}

cout<<"Total observation time: "<<totalT * (24.0 / 360.0)<<" hours."<<endl;
cout<<"Total Acceptance over "<<totalT * (24.0 / 360.0)<<" hours: "<<totalAcc<<endl;
cout<<"Duty Cycle: "<<totalT * (24.0 / 360.0) * (1/87600.0) * 100.<<" percent"<<endl;

TCanvas *skyProjC = new TCanvas("skyProjC","Skymap Projection",1500,750);


TMarker *galMarks[10];

for(int i = 0; i < 10; i++) {
	galMarks[i] = new TMarker(0.,0., 43);
	galMarks[i]->SetMarkerSize(1.5);
}

galMarks[0]->SetX(0); //galactic center
galMarks[0]->SetY(0);

galMarks[1]->SetX(155.711); //TXS
galMarks[1]->SetY(-28.0311);

galMarks[2]->SetX(-53.1613); //MRK 501
galMarks[2]->SetY(40.6421);

galMarks[3]->SetX(-75.9516); //MRK 421
galMarks[3]->SetY(81.5564);

galMarks[4]->SetX(32.5099); //Virgo
galMarks[4]->SetY(60.2204);

galMarks[5]->SetX(-75.9174); //cygnus A
galMarks[5]->SetY(6.20106);

galMarks[6]->SetX(48.4862); // Cen A
galMarks[6]->SetY(20.0392);

galMarks[7]->SetX(123); //Auger Dipole
galMarks[7]->SetY(-8);

galMarks[8]->SetX(83.9959); //Fornax
galMarks[8]->SetY(-66.0235);

galMarks[9]->SetX(-152); //TA Hotspot
galMarks[9]->SetY(47);

TText *labels[10];
	
for(int i = 0; i < 10; i++) {
	labels[i] = new TText();
	labels[i]->SetTextSize(20);
	labels[i]->SetTextFont(43);
	labels[i]->SetTextAlign(22);
}
float conv=TMath::Pi()/180; 
float la, lo, x, yy, z;
const int Nl = 5; // Number of drawn latitudes
const int NL = 5; // Number of drawn longitudes
int       M  = 30;

   TGraph  *latitudes[Nl];
   TGraph  *longitudes[NL];

   for (int j=0;j<Nl;++j) {
      latitudes[j]=new TGraph();
      la = -90+180/(Nl-1)*j;
      for (int i=0;i<M+1;++i) {
         lo = -180+360/M*i;
         z  = sqrt(1+cos(la*conv)*cos(lo*conv/2));
         x  = 180*cos(la*conv)*sin(lo*conv/2)/z;
         yy  = 90*sin(la*conv)/z;
         latitudes[j]->SetPoint(i,x,yy);
      }
   } 
  
   for (int j=0;j<NL;++j) {
      longitudes[j]=new TGraph();
      lo = -180+360/(NL-1)*j;
      for (int i=0;i<M+1;++i) {
         la = -90+180/M*i;
         z  = sqrt(1+cos(la*conv)*cos(lo*conv/2));
         x  = 180*cos(la*conv)*sin(lo*conv/2)/z;
         yy  = 90*sin(la*conv)/z;
         longitudes[j]->SetPoint(i,x,yy);
      }
   }

labels[0]->SetText(-29, -5, "Galactic Center");
labels[1]->SetText(-55., 5.0, "Cygnus A");
labels[2]->SetText(-35., 35., "MRK 501");
labels[3]->SetText(-97., 85., "MRK 421");
labels[4]->SetText(-125., 45., "TA Hotspot");
labels[5]->SetText(36., 55, "Virgo");
labels[6]->SetText(48, 25, "Cen A");
labels[7]->SetText(150, -10, "Auger Dipole");
labels[8]->SetText(135, -35, "TXS 0506+056");
labels[9]->SetText(75, -72, "Fornax");

gPad->SetLogz(0);
gStyle->SetPalette(56);
skyProjC->SetRightMargin(0.2);
skymapFullProjection->GetXaxis()->SetTitle("Galactic Longitude [deg]");
skymapFullProjection->GetYaxis()->SetTitle("Galactic Latitude [deg]");
skymapFullProjection->GetZaxis()->SetTitle("Acceptance [cm^{2} s]");
skymapFullProjection->Draw("z aitoff");

TPad *pad2 = new TPad("pad2","",0,0,1,1);
pad2->SetFillStyle(4000);
pad2->SetFillColor(0);
pad2->SetBorderSize(0);
pad2->SetFrameBorderMode(0);
pad2->SetFrameLineColor(0); 
pad2->SetFrameBorderMode(0);
pad2->Draw();
pad2->cd();
pad2->Range(-231,-111.875,283,111.875);

for (int j=0;j<Nl;++j) latitudes[j]->Draw("l");
for (int j=0;j<NL;++j) longitudes[j]->Draw("l");

for(int i = 0; i < 10; i++) {
	galMarks[i]->Draw();
	labels[i]->Draw();
}

return (dIntegratedAcceptance*dConversion);
}
*/
void CalculateAcceptanceVsLowerFoV(TH1D *hTau)
{

  bCombined = kFALSE;

  MaxElevation = 10; //elevation angle (determines path through Earth; 
  DeltaAngle = 0.05; //steps in azimuth and elevation 
  yMin = 0;
  yDelta = 5; 


  iMirrorSize = 2;
  dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 

  dMinLength = 0.3; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.

  tanFoV = tan(10/180.*pi); //Field of view of telescope above the horizon
  

  Double_t dPreFactor = pow(10,(dMaxEnu+dMinEnu)) / (pow(10,dMaxEnu)-pow(10,dMinEnu));
  dPreFactor *= 3;//three neutrino flavours

  iConfig = 2; //telescope altitude


  TMultiGraph *mgDiffAcceptance = new TMultiGraph();
  TMultiGraph *mgAcceptance = new TMultiGraph();
  TLegend *legAcceptance = new TLegend(0.6,0.54,0.91,0.91,"FoV below horizon");
  legAcceptance->SetTextSize(0.05);

  for(int b = 0;b<2;b++)
  {
    bFluorescence=b;
    TGraph *grAcceptvsFoV = new TGraph();
    grAcceptvsFoV->SetLineWidth(3);
    grAcceptvsFoV->SetLineColor(kBlue+3);
    mgAcceptance->Add(grAcceptvsFoV);

     if(bFluorescence)
      {
        
        yMax = 100;
        grAcceptvsFoV->SetLineStyle(9);
      }
    else
      {
         yMax = 400;
       }

    for(int f = 0;f<6;f++)
     {
        dFoVBelow = (f+0.01)/180.*pi;

       TGraph *grDiffAcceptance = new TGraph();

       if(bFluorescence)
        {
           grDiffAcceptance->SetLineStyle(2);
           TString title;
           title.Form("%i#circ",(f));
           legAcceptance->AddEntry(grDiffAcceptance,title.Data(),"p"); 
        }


       grDiffAcceptance->SetMarkerStyle(marker[f]);
       grDiffAcceptance->SetMarkerSize(markerSize[f]);
       grDiffAcceptance->SetMarkerColor(iColors[f]);
       grDiffAcceptance->SetLineColor(iColors[f]);
       mgDiffAcceptance->Add(grDiffAcceptance,"lp");
    
       Double_t dAcceptance = CalculateAcceptance(dMinEnu,dMaxEnu,grDiffAcceptance,hTau);
     //  cout<<"Sensitivity "<<dPreFactor/dAcceptance<<endl;
       grAcceptvsFoV->SetPoint(f,f*2+3,dAcceptance);
     }//looping over different FoV
   } //looping over Fluo and Cherenkov
  //Done with calculating the sensitivity

  TCanvas *cAcceptvsFoV = new TCanvas("cAcceptvsFoV","Acceptance vs. lower FoV",750,500);
  cAcceptvsFoV->Draw();
  mgAcceptance->Draw("alp");
  mgAcceptance->GetXaxis()->SetTitle("vertical field of view [degrees]");
  mgAcceptance->GetYaxis()->SetTitle("acceptance [cm^{2} sr]");
  mgAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgAcceptance->GetXaxis()->SetLabelSize(0.04);
  

  TCanvas *cDiffAcceptDistance = new TCanvas("cDiffAcceptDistance","Acceptance vs. Distance",750,500);
  cDiffAcceptDistance->Draw();
  mgDiffAcceptance->Draw("a");
  mgDiffAcceptance->GetXaxis()->SetTitle("distance of tau emergence from telescope [km]");
  mgDiffAcceptance->GetYaxis()->SetTitle("radial acceptance [cm^{2} sr]");
  mgDiffAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetLabelSize(0.04);
  legAcceptance->Draw();

  
}


void CalculateAcceptanceVsUpperFoV(TH1D *hTau)
{

  bCombined = kFALSE;

  MaxElevation = 10; //elevation angle (determines path through Earth; 
  DeltaAngle = 0.05; //steps in azimuth and elevation 
  yMin = 0;
  yDelta = 5; 


  iMirrorSize = 2;
  dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 

  dMinLength = 0.3; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.

  Double_t dPreFactor = pow(10,(dMaxEnu+dMinEnu)) / (pow(10,dMaxEnu)-pow(10,dMinEnu));
  dPreFactor *= 3;//three neutrino flavours

  iConfig = 2; //telescope altitude

  dFoVBelow =   asin(REarth/(REarth+DetectorAltitude[iConfig]));
  //dFoVBelow =  3.0/180.*pi; 

  TMultiGraph *mgDiffAcceptance = new TMultiGraph();
  TMultiGraph *mgAcceptance = new TMultiGraph();
  TLegend *legAcceptance = new TLegend(0.6,0.54,0.91,0.91,"FoV above horizon");
  legAcceptance->SetTextSize(0.05);

  for(int b = 0;b<2;b++)
  {
    bFluorescence=b;
    TGraph *grAcceptvsFoV = new TGraph();
    grAcceptvsFoV->SetLineWidth(3);
    grAcceptvsFoV->SetLineColor(kBlue+3);
    mgAcceptance->Add(grAcceptvsFoV);

     if(bFluorescence)
      {
        
        yMax = 100;
        grAcceptvsFoV->SetLineStyle(9);
      }
    else
      {
         yMax = 400;
       }

    for(int f = 0;f<6;f++)
     {
       tanFoV = tan((f*0.5+0.01)/180.*pi);

       TGraph *grDiffAcceptance = new TGraph();

       if(bFluorescence)
        {
           grDiffAcceptance->SetLineStyle(2);
           TString title;
           title.Form("%0.1f#circ",(f*0.5));
           legAcceptance->AddEntry(grDiffAcceptance,title.Data(),"p"); 
        }


       grDiffAcceptance->SetMarkerStyle(marker[f]);
       grDiffAcceptance->SetMarkerSize(markerSize[f]);
       grDiffAcceptance->SetMarkerColor(iColors[f]);
       grDiffAcceptance->SetLineColor(iColors[f]);
       mgDiffAcceptance->Add(grDiffAcceptance,"lp");
    
       Double_t dAcceptance = CalculateAcceptance(dMinEnu,dMaxEnu,grDiffAcceptance,hTau);
       cout<<"Sensitivity "<<dPreFactor/dAcceptance<<endl;
       grAcceptvsFoV->SetPoint(f,f*2+3,dAcceptance);
     }//looping over different FoV
   } //looping over Fluo and Cherenkov
  //Done with calculating the sensitivity

  TCanvas *cAcceptvsFoV = new TCanvas("cAcceptvsFoV","Acceptance vs. FoV",750,500);
  cAcceptvsFoV->Draw();
  mgAcceptance->Draw("alp");
  mgAcceptance->GetXaxis()->SetTitle("vertical field of view [degrees]");
  mgAcceptance->GetYaxis()->SetTitle("acceptance [cm^{2} sr]");
  mgAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgAcceptance->GetXaxis()->SetLabelSize(0.04);
  

  TCanvas *cDiffAcceptDistance = new TCanvas("cDiffAcceptDistance","Acceptance vs. Distance",750,500);
  cDiffAcceptDistance->Draw();
  mgDiffAcceptance->Draw("a");
  mgDiffAcceptance->GetXaxis()->SetTitle("distance of tau emergence from telescope [km]");
  mgDiffAcceptance->GetYaxis()->SetTitle("radial acceptance [cm^{2} sr]");
  mgDiffAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetLabelSize(0.04);
  legAcceptance->Draw();

  
}

void CalculateAcceptanceVsEnergy(TH1D *hTau)
{

  bCombined = kFALSE;

  MaxElevation = 10; //elevation angle (determines path through Earth; 
  DeltaAngle = 0.05; //steps in azimuth and elevation 
  yMin = 0;
  yDelta = 5; 

  iConfig = 2; //telescope altitude
  dFoVBelow = asin(REarth/(REarth+DetectorAltitude[iConfig]));
  tanFoV = tan(5./180.*pi);
  dMinLength = 0.3; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.

  iMirrorSize = 2;
  dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 

  Double_t dLogEnergyStep = 0.2;
  Double_t dHalfEnergyBinWidth =1/2.; //in log
  Double_t logEmin = 10.0; //was 7




  TMultiGraph *mgDiffAcceptance = new TMultiGraph();
  TMultiGraph *mgAcceptance = new TMultiGraph();
  TLegend *legAcceptance = new TLegend(0.64,0.64,0.95,0.95,"minimum image length");
  legAcceptance->SetTextSize(0.05);

 for(int b = 0;b<2;b++)
  {
    bFluorescence=b;
    TGraph *grAcceptvsImageSize = new TGraph();
    grAcceptvsImageSize->SetLineWidth(3);
    grAcceptvsImageSize->SetLineColor(kBlue+3);
    mgAcceptance->Add(grAcceptvsImageSize);

     if(bFluorescence)
      {
        
        yMax = 100;
        grAcceptvsImageSize->SetLineStyle(9);
      }
    else
      {
         yMax = 400;
       }

    for(int f = 0;f<1;f++) 
      {
       dMinEnu=logEmin+f*dLogEnergyStep-dHalfEnergyBinWidth;
       dMaxEnu=logEmin+f*dLogEnergyStep+dHalfEnergyBinWidth;
       Double_t dPreFactor = pow(10,dMaxEnu+dMinEnu) / (pow(10,dMaxEnu)-pow(10,dMinEnu));
       dPreFactor *= 3;//three neutrino flavours
       

       TGraph *grDiffAcceptance = new TGraph();
       if(bFluorescence)
        {
           grDiffAcceptance->SetLineStyle(2);
           TString title;
           title.Form("%0.1f GeV",logEmin+f*dLogEnergyStep);
           legAcceptance->AddEntry(grDiffAcceptance,title.Data(),"p"); 
        }
       
       grDiffAcceptance->SetMarkerStyle(marker[f]);
       grDiffAcceptance->SetMarkerSize(markerSize[f]);
       grDiffAcceptance->SetMarkerColor(iColors[f]);
       grDiffAcceptance->SetLineColor(iColors[f]);
       grDiffAcceptance->SetLineWidth(2);
       mgDiffAcceptance->Add(grDiffAcceptance,"lp");
    
       Double_t dAcceptance = CalculateAcceptance(dMinEnu,dMaxEnu,grDiffAcceptance,hTau);
       cout<<"Sensitivity "<<dPreFactor/dAcceptance<<endl;
       grAcceptvsImageSize->SetPoint(f,dMinLength,dAcceptance);
      } //looping over different energies
   } //looping over Fluo and Cherenkov

  TCanvas *cAcceptvsImageSize = new TCanvas("cAcceptvsImageSize","Acceptance vs. Image Size",750,500);
  cAcceptvsImageSize->Draw();
  mgAcceptance->Draw("alp");
  mgAcceptance->GetXaxis()->SetTitle("Energy [log10(GeV)]");
  mgAcceptance->GetYaxis()->SetTitle("acceptance [cm^{2} sr]");
  mgAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgAcceptance->GetXaxis()->SetLabelSize(0.04);
  

  TCanvas *cDiffAcceptDistance = new TCanvas("cDiffAcceptDistance","Acceptance vs. Distance",750,500);
  cDiffAcceptDistance->Draw();
  mgDiffAcceptance->Draw("alp");
  mgDiffAcceptance->GetXaxis()->SetTitle("distance of tau emergence from telescope [km]");
  mgDiffAcceptance->GetYaxis()->SetTitle("radial acceptance [cm^{2} sr]");
  mgDiffAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetLabelSize(0.04);
  legAcceptance->Draw();


}
void CalculateAcceptanceVsImageLength(TH1D *hTau)
{

  bCombined = kFALSE;

  MaxElevation = 10; //elevation angle (determines path through Earth; 
  DeltaAngle = 0.05; //steps in azimuth and elevation 
  yMin = 0;
  yDelta = 5; 

  iConfig = 2; //telescope altitude
  dFoVBelow = 3./180*pi; //asin(REarth/(REarth+DetectorAltitude[iConfig]));
  tanFoV = tan(2./180.*pi);

  iMirrorSize = 2;
  dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 



  Double_t dPreFactor = pow(10,dMaxEnu+dMinEnu) / (pow(10,dMaxEnu)-pow(10,dMinEnu));
  dPreFactor *= 3;//three neutrino flavours


  TMultiGraph *mgDiffAcceptance = new TMultiGraph();
  TMultiGraph *mgAcceptance = new TMultiGraph();
  TLegend *legAcceptance = new TLegend(0.64,0.64,0.95,0.95,"minimum image length");
  legAcceptance->SetTextSize(0.05);

  for(int b = 0;b<2;b++)
  {
    bFluorescence=b;
    TGraph *grAcceptvsImageSize = new TGraph();
    grAcceptvsImageSize->SetLineWidth(3);
    grAcceptvsImageSize->SetLineColor(kBlue+3);
    mgAcceptance->Add(grAcceptvsImageSize);

     if(bFluorescence)
      {
        
        yMax = 100;
        grAcceptvsImageSize->SetLineStyle(9);
      }
    else
      {
         yMax = 400;
       }

    for(int f = 0;f<5;f++) 
      {
        dMinLength = 0.2*f+0.1; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.
       TGraph *grDiffAcceptance = new TGraph();
       if(bFluorescence)
        {
           grDiffAcceptance->SetLineStyle(2);
           TString title;
           title.Form("%0.1f#circ",dMinLength);
           legAcceptance->AddEntry(grDiffAcceptance,title.Data(),"p"); 
        }
       
       grDiffAcceptance->SetMarkerStyle(marker[f]);
       grDiffAcceptance->SetMarkerSize(markerSize[f]);
       grDiffAcceptance->SetMarkerColor(iColors[f]);
       grDiffAcceptance->SetLineColor(iColors[f]);
       grDiffAcceptance->SetLineWidth(2);
       mgDiffAcceptance->Add(grDiffAcceptance,"lp");
    
       Double_t dAcceptance = CalculateAcceptance(dMinEnu,dMaxEnu,grDiffAcceptance,hTau);
       cout<<"Sensitivity "<<dPreFactor/dAcceptance<<endl;
       grAcceptvsImageSize->SetPoint(f,dMinLength,dAcceptance);
      } //looping over different minimum image size
   } //looping over Fluo and Cherenkov

  TCanvas *cAcceptvsImageSize = new TCanvas("cAcceptvsImageSize","Acceptance vs. Image Size",750,500);
  cAcceptvsImageSize->Draw();
  mgAcceptance->Draw("alp");
  mgAcceptance->GetXaxis()->SetTitle("minimum image length [degrees]");
  mgAcceptance->GetYaxis()->SetTitle("acceptance [cm^{2} sr]");
  mgAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgAcceptance->GetXaxis()->SetLabelSize(0.04);
  

  TCanvas *cDiffAcceptDistance = new TCanvas("cDiffAcceptDistance","Acceptance vs. Distance",750,500);
  cDiffAcceptDistance->Draw();
  mgDiffAcceptance->Draw("alp");
  mgDiffAcceptance->GetXaxis()->SetTitle("distance of tau emergence from telescope [km]");
  mgDiffAcceptance->GetYaxis()->SetTitle("radial acceptance [cm^{2} sr]");
  mgDiffAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetLabelSize(0.04);
  legAcceptance->Draw();


}

void CalculateAcceptanceVsTelescopeHeight(TH1D *hTau)
{

  bCombined = kFALSE;

  MaxElevation = 10; //elevation angle (determines path through Earth; 
  DeltaAngle = 0.05; //steps in azimuth and elevation 
  yMin = 0;
  yDelta = 5; 

  tanFoV = tan(10./180.*pi);
  dFoVBelow = asin(REarth/(REarth+DetectorAltitude[3]));

  iMirrorSize = 2;
  dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 

  dMinLength = 0.3; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.

  Double_t dPreFactor = pow(10,(dMaxEnu+dMinEnu)) / (pow(10,dMaxEnu)-pow(10,dMinEnu));
  dPreFactor *= 3;//three neutrino flavours


  TMultiGraph *mgDiffAcceptance = new TMultiGraph();
  TMultiGraph *mgAcceptance = new TMultiGraph();
  TLegend *legAcceptance = new TLegend(0.6,0.54,0.91,0.91,"telescope height");
  legAcceptance->SetTextSize(0.05);

  for(int b = 0;b<2;b++)
  {
    bFluorescence=b;
    TGraph *grAcceptvsHeight = new TGraph();
    grAcceptvsHeight->SetLineWidth(3);
    grAcceptvsHeight->SetLineColor(kBlue+3);
    mgAcceptance->Add(grAcceptvsHeight);

     if(bFluorescence)
      {
        
        yMax = 100;
        grAcceptvsHeight->SetLineStyle(9);
      }
    else
      {
         yMax = 400;
       }

    for(unsigned int f = 0;f<sizeof(DetectorAltitude)/sizeof(*DetectorAltitude);f++)
     {
      iConfig = f; //telescope altitude
      TGraph *grDiffAcceptance = new TGraph();
         if(bFluorescence)
          {
            grDiffAcceptance->SetLineStyle(2);
            TString title;
            title.Form("%i km",f);
            legAcceptance->AddEntry(grDiffAcceptance,title.Data(),"p"); 
          }

      grDiffAcceptance->SetMarkerStyle(marker[f]);
      grDiffAcceptance->SetMarkerSize(markerSize[f]);
      grDiffAcceptance->SetMarkerColor(iColors[f]);
      grDiffAcceptance->SetLineColor(iColors[f]);
      mgDiffAcceptance->Add(grDiffAcceptance,"lp");
    
      Double_t dAcceptance = CalculateAcceptance(dMinEnu,dMaxEnu,grDiffAcceptance,hTau);
      cout<<"Sensitivity "<<dPreFactor/dAcceptance<<endl;
      grAcceptvsHeight->SetPoint(f,DetectorAltitude[f],dAcceptance);
     }//looping over different FoV
   } //looping over Fluo and Cherenkov
  //Done with calculating the sensitivity

  TCanvas *cAcceptvsHeight = new TCanvas("cAcceptvsHeight","Acceptance vs. Telescope Height",750,500);
  cAcceptvsHeight->Draw();
  mgAcceptance->Draw("alp");
  mgAcceptance->GetXaxis()->SetTitle("telescope height [km]");
  mgAcceptance->GetYaxis()->SetTitle("acceptance [cm^{2} sr]");
  mgAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgAcceptance->GetXaxis()->SetLabelSize(0.04);
  

  TCanvas *cDiffAcceptDistance = new TCanvas("cDiffAcceptDistance","Acceptance vs. Distance",750,500);
  cDiffAcceptDistance->Draw();
  mgDiffAcceptance->Draw("a");
  mgDiffAcceptance->GetXaxis()->SetTitle("distance of tau emergence from telescope [km]");
  mgDiffAcceptance->GetYaxis()->SetTitle("radial acceptance [cm^{2} sr]");
  mgDiffAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetLabelSize(0.04);
  legAcceptance->Draw();


}

void CalculateAcceptanceVsThreshold(TH1D *hTau)
{

  bCombined = kFALSE;

  MaxElevation = 10; //elevation angle (determines path through Earth; 
  DeltaAngle = 0.05; //steps in azimuth and elevation 
  yMin = 0;
  yDelta = 5; 

  tanFoV = tan(2./180.*pi);

  iConfig = 2; //telescope altitude
  dFoVBelow = 3./180*pi; 
 // dFoVBelow = asin(REarth/(REarth+DetectorAltitude[iConfig]));

  dMinLength = 0.3; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.

  Double_t dPreFactor = pow(10,(dMaxEnu+dMinEnu)) / (pow(10,dMaxEnu)-pow(10,dMinEnu));
  dPreFactor *= 3;//three neutrino flavours


  TMultiGraph *mgDiffAcceptance = new TMultiGraph();
  TMultiGraph *mgAcceptance = new TMultiGraph();
  TLegend *legAcceptance = new TLegend(0.6,0.54,0.91,0.91,"mirror size");
  legAcceptance->SetTextSize(0.05);

  for(int b = 0;b<2;b++)
  {
    bFluorescence=b;
    TGraph *grAcceptvsMirror = new TGraph();
    grAcceptvsMirror->SetLineWidth(3);
    grAcceptvsMirror->SetLineColor(kBlue+3);
    mgAcceptance->Add(grAcceptvsMirror);

     if(bFluorescence)
      {
        
        yMax = 100;
        grAcceptvsMirror->SetLineStyle(9);
      }
    else
      {
         yMax = 400;
       }

  for(unsigned int f = 0;f<4;f++)
   {
     iMirrorSize = f;
     dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 

     TGraph *grDiffAcceptance = new TGraph();

     if(bFluorescence)
        {
           grDiffAcceptance->SetLineStyle(2);
           TString title;
           title.Form("%0.0f m^{2}",dMirrorA[f]);
           legAcceptance->AddEntry(grDiffAcceptance,title.Data(),"p"); 
        }

     
    grDiffAcceptance->SetMarkerStyle(marker[f]);
    grDiffAcceptance->SetMarkerSize(markerSize[f]);
    grDiffAcceptance->SetMarkerColor(iColors[f]);
    grDiffAcceptance->SetLineColor(iColors[f]);
    mgDiffAcceptance->Add(grDiffAcceptance,"lp");
    
    Double_t dAcceptance = CalculateAcceptance(dMinEnu,dMaxEnu,grDiffAcceptance,hTau);
    cout<<"Sensitivity "<<dPreFactor/dAcceptance<<endl;
    grAcceptvsMirror->SetPoint(f,dMirrorA[iMirrorSize],dAcceptance);
   }//looping over different FoV
  } //looping over Fluo and Cherenkov
  //Done with calculating the sensitivity

  TCanvas *cAcceptvsMirrorArea = new TCanvas("cAcceptvsImageSize","Acceptance vs. Mirror Area",750,500);
  cAcceptvsMirrorArea->Draw();
  mgAcceptance->Draw("alp");
  mgAcceptance->GetXaxis()->SetTitle("mirror area [m^{2}]");
  mgAcceptance->GetYaxis()->SetTitle("acceptance [cm^{2} sr]");
  mgAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgAcceptance->GetXaxis()->SetLabelSize(0.04);
  

  TCanvas *cDiffAcceptDistance = new TCanvas("cDiffAcceptDistance","Acceptance vs. Distance",750,500);
  cDiffAcceptDistance->Draw();
  mgDiffAcceptance->Draw("a");
  mgDiffAcceptance->GetXaxis()->SetTitle("distance of tau emergence from telescope [km]");
  mgDiffAcceptance->GetYaxis()->SetTitle("radial acceptance [cm^{2} sr]");
  mgDiffAcceptance->GetYaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetYaxis()->SetLabelSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetTitleSize(0.04);
  mgDiffAcceptance->GetXaxis()->SetLabelSize(0.04);
  legAcceptance->Draw();


}
////////////////////////////////////////////////////////////////////
//
//
// Calculate Integral Sensitiivty
//
//
void CalculateIntegralSensitivity(TH1D *hTau)
{

    Double_t dLogEnergyStep = 0.2;
    Double_t logEmin = 6;
    Double_t logEmax = 10;

    if(bFluorescence)
       {
          yMin = 1;
          yMax = 100;
          yDelta = 5;
          MaxElevation = 10; //elevation angle (determines path through Earth;
          DeltaAngle = 0.3; //steps in azimuth and elevation 
       }
    else
       {
          yMin = 10;
          yMax = 400;
          yDelta = 10;
          MaxElevation = 10; //elevation angle (determines path through Earth;
          DeltaAngle = 0.3; //steps in azimuth and elevation 
       }


    dMinimumNumberPhotoelectrons = 20;
    tanFoV = tan(5./180.*pi);
    dMinLength = 0.3; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.

    TGraph *grSensitivity = new TGraph();

    TGraph *grDiffAcceptance = new TGraph();

  TCanvas *cIntSensitivity = new TCanvas("cIntSensitivity","Integral Sensitivity",750,500);
  cIntSensitivity->Draw();
  cIntSensitivity->SetLogy();
  cIntSensitivity->SetLogx();
  

    //Move in steps from highest energy to lowest adding acceptance
    Double_t dLogE = logEmax;
    Int_t n=0;
    Double_t dIntegratedAcceptance=0;
    while(dLogE>logEmin)
       {
          Double_t dAcceptance = CalculateAcceptance(dLogE,dLogE+dLogEnergyStep,grDiffAcceptance,hTau);
          Double_t dEIndexed = pow(10,-dLogE*nuIndex);
          dIntegratedAcceptance+=dAcceptance
             *dEIndexed*(pow(10,dLogE+dLogEnergyStep)-pow(10,dLogE));

          Double_t dnuFnu = 3 * pow(10,dLogE*2) * dEIndexed / dIntegratedAcceptance;
          cout<<"Energy "<<dLogE<<" integrated acceptance:  "<<dIntegratedAcceptance<<" converted to nuFnu: "<<dnuFnu<<" for power law with index -"<<nuIndex<<endl;
          grSensitivity->SetPoint(n,pow(10,dLogE),dnuFnu);

          n++;
          dLogE-=dLogEnergyStep;
          cIntSensitivity->cd();
          grSensitivity->Draw("alp");
          cIntSensitivity->Modified();
          cIntSensitivity->Update();
       }

  grSensitivity->SetLineWidth(3);
  grSensitivity->SetLineColor(kBlue+3);
  grSensitivity->GetXaxis()->SetTitle("energy [GeV]");
  grSensitivity->GetYaxis()->SetTitle("E^{2} dN/dE [ GeV cm^{-2} s^{-1} sr^{-1} ]");
  grSensitivity->GetYaxis()->SetTitleSize(0.04);
  grSensitivity->GetYaxis()->SetLabelSize(0.04);
  grSensitivity->GetXaxis()->SetTitleSize(0.04);
  grSensitivity->GetXaxis()->SetLabelSize(0.04);
}

////////////////////////////////////////////////////////////////////
//
//
// Calculate Differential Sensitiivty
//
//
void CalculateDifferentialSensitivity(TH1D *hTau)
{

    Double_t dLogEnergyStep = 0.2; //0.2
    Double_t dHalfEnergyBinWidth =1/2.; //in log was 1/2
    Double_t logEmin = 6; //7
    Double_t logEmax = 10.5; //11

    bCombined = kTRUE;
    yMin = 5; //5
    yMax = 500; //500
    yDelta = 5; //5
    MaxElevation = 20; //elevation angle (determines path through Earth;
    DeltaAngle = 0.1; //steps in azimuth and elevation 

    iConfig = 2; //telescope altitude
  
    //exposure
    Double_t dExposure=10*365*24*3600*0.20; //exposure time 10 years in seconds with 20% duty cycle

    Double_t dFoV = 2;  //test 0, 1, 2, 10
    tanFoV = tan(dFoV/180.*pi);
    //dFoVBelow = asin(REarth/(REarth+DetectorAltitude[iConfig]));
    dFoVBelow =  3/180.*pi; 
    iMirrorSize = 2;
    dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 

    dMinLength = 0.3; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.
    TGraph *grDiffAcceptance = new TGraph();

    TGraph *grSensitivity = new TGraph();
    grSensitivity->SetLineWidth(3);
    grSensitivity->SetLineColor(kBlue+3);

    TGraph *grSensitivityNuCommunity = new TGraph();
    grSensitivityNuCommunity->SetLineWidth(3);
    grSensitivityNuCommunity->SetLineColor(kRed+3);



    TCanvas *cDiffSensitivity = new TCanvas("cDiffSensitivity","Differential Sensitivity",750,500);
    cDiffSensitivity->Draw();
    cDiffSensitivity->SetLogy();
    cDiffSensitivity->SetLogx();

    TGraph *grAcceptance = new TGraph();
    grAcceptance->SetLineWidth(3);
    grAcceptance->SetLineColor(kBlue+3);

    TGraph *grAcceptanceMonoEnergy = new TGraph();
    grAcceptanceMonoEnergy->SetLineWidth(3);
    grAcceptanceMonoEnergy->SetLineColor(kRed+3);

    TCanvas *cAcceptance = new TCanvas("cAcceptance","Acceptance",750,500);
    cAcceptance->Draw();
    cAcceptance->SetLogy();
    cAcceptance->SetLogx();
  
    TCanvas *cTriggeredAzimuthAngles = new TCanvas("cTriggeredAzimuthAngles","Triggered Azimuth Angles",750,500);
    hTriggeredAzimuthAngles->Draw("HIST");

  


    //Move in steps from lowest to highes energy
    Double_t dLogE = logEmin;
    Int_t n=0;
    while(dLogE<=logEmax)
       {
          bMonoNu = kFALSE; //calculate the acceptance averaged over Enu bin
          Double_t dAcceptance = CalculateAcceptance(dLogE-dHalfEnergyBinWidth,dLogE+dHalfEnergyBinWidth,grDiffAcceptance,hTau);
          //~ Double_t dAcceptance = CalculateAcceptance(9.0,9.1,grDiffAcceptance,hTau);
          //~ Double_t dAcceptance = CalculateAcceptance2(6.0,10.0,grDiffAcceptance,hTau);
          
          if(dAcceptance>1)
           {
              grAcceptance->SetPoint(n,pow(10,dLogE),dAcceptance);

              Double_t dnuFnu = 3 * 2.44 * pow(10,dLogE*2) / dAcceptance / dExposure / (pow(10,dLogE+dHalfEnergyBinWidth)- pow(10,dLogE-dHalfEnergyBinWidth));
              cout<<"Energy "<<dLogE-dHalfEnergyBinWidth<<" to "<<dLogE+dHalfEnergyBinWidth<<" acceptance:  "<<dAcceptance<<" nuFnu: "<<dnuFnu<<" for power law with index -"<<nuIndex<<endl;
              grSensitivity->SetPoint(n,pow(10,dLogE),dnuFnu);
              cDiffSensitivity->cd();
              grSensitivity->Draw("alp");

              //Calculate the Sensitivity as done by the Nu Community
              bMonoNu = kTRUE; //calculate the acceptance at Enu bin center
              //~ dAcceptance = CalculateAcceptance(dLogE-dHalfEnergyBinWidth,dLogE+dHalfEnergyBinWidth,grDiffAcceptance,hTau);
              grAcceptanceMonoEnergy->SetPoint(n,pow(10,dLogE),dAcceptance);
              bMonoNu = kFALSE;
              dnuFnu = 3 * 2.44 / dAcceptance / dExposure / log(10) / (2*dHalfEnergyBinWidth) * pow(10,dLogE); //2.44 is from Feldman Cousin 90% confidence upper limit
              cout<<"NuCommunity Definition: "<<dnuFnu<<" acceptance at center energy: "<<dAcceptance<<endl;
              grSensitivityNuCommunity->SetPoint(n,pow(10,dLogE),dnuFnu);
              grSensitivityNuCommunity->Draw("lp");

              cDiffSensitivity->Modified();
              cDiffSensitivity->Update();

              cAcceptance->cd();
              grAcceptance->Draw("alp");
              grAcceptanceMonoEnergy->Draw("lp");
              cAcceptance->Modified();
              cAcceptance->Update();

              cTriggeredAzimuthAngles->cd();
              cTriggeredAzimuthAngles->Modified();
              cTriggeredAzimuthAngles->Update();

              n++;
            }
          dLogE+=dLogEnergyStep;
       }


  cDiffSensitivity->cd();
  TLegend *legend = new TLegend(0.53,0.7,0.75,0.88);
  TString legstr;
  legend->AddEntry(grSensitivity,"Integration","l");
  legend->AddEntry(grSensitivityNuCommunity,"Approximation","l");
  legend->Draw();

  grSensitivity->GetXaxis()->SetTitle("energy [GeV]");
  grSensitivity->GetYaxis()->SetTitle("E^{2} dN/dE [ GeV cm^{-2} s^{-1} sr^{-1} ]");
  grSensitivity->GetYaxis()->SetTitleSize(0.04);
  grSensitivity->GetYaxis()->SetLabelSize(0.04);
  grSensitivity->GetXaxis()->SetTitleSize(0.04);
  grSensitivity->GetXaxis()->SetLabelSize(0.04);

  cAcceptance->cd();
  legend = new TLegend(0.755,0.49,0.84,0.63);
  legend->AddEntry(grAcceptance,"Averaged","l");
  legend->AddEntry(grAcceptanceMonoEnergy,"Mono","l");
  legend->Draw();

  grAcceptance->GetXaxis()->SetTitle("energy [GeV]");
  grAcceptance->GetYaxis()->SetTitle("acceptance [ cm^{2} sr ]");
  grAcceptance->GetYaxis()->SetTitleSize(0.04);
  grAcceptance->GetYaxis()->SetLabelSize(0.04);
  grAcceptance->GetXaxis()->SetTitleSize(0.04);
  grAcceptance->GetXaxis()->SetLabelSize(0.04);

  hTriggeredAzimuthAngles->SetLineWidth(3);
  hTriggeredAzimuthAngles->SetLineColor(kBlue+3);
  hTriggeredAzimuthAngles->GetYaxis()->SetTitleSize(0.04);
  hTriggeredAzimuthAngles->GetYaxis()->SetLabelSize(0.04);
  hTriggeredAzimuthAngles->GetXaxis()->SetTitleSize(0.04);
  hTriggeredAzimuthAngles->GetXaxis()->SetLabelSize(0.04);
  hTriggeredAzimuthAngles->Scale(1.0/hTriggeredAzimuthAngles->Integral(),"nosw2");

  TString Filename;
  Filename.Form("SensitivityResults/StudyHighestEnergies/DifferentialSensitivityTrinity_NuTauSim_%ikmAboveGround_%0.0fsqrmMirror_%0.1fdegUpperFoV_%0.1fdegLowerFoV_%0.1fdegMinShowerLength.root",iConfig,dMirrorA[iMirrorSize],dFoV,dFoVBelow/pi*180.,dMinLength);
  cDiffSensitivity->SaveAs(Filename.Data());
  Filename.Form("SensitivityResults/StudyHighestEnergies/AcceptanceTrinity_NuTauSim_%ikmAboveGround_%0.0fsqrmMirror_%0.1fdegUpperFoV_%0.1fdegLowerFoV_%0.1fdegMinShowerLength.root",iConfig,dMirrorA[iMirrorSize],dFoV,dFoVBelow/pi*180.,dMinLength);
  cAcceptance->SaveAs(Filename.Data());
  Filename.Form("SensitivityResults/StudyHighestEnergies/TriggeredAzimuthTrinity_NuTauSim_%ikmAboveGround_%0.0fsqrmMirror_%0.1fdegUpperFoV_%0.1fdegLowerFoV_%0.1fdegMinShowerLength.root",iConfig,dMirrorA[iMirrorSize],dFoV,dFoVBelow/pi*180.,dMinLength);
  cTriggeredAzimuthAngles->SaveAs(Filename.Data());

  Double_t *dE = grSensitivity->GetX(); 
  Double_t *dF = grSensitivity->GetY(); 
  for(int i=0;i<grSensitivity->GetN();i++)
     {
       cout<<dE[i]<<"  "<<dF[i]<<endl;;
     }

}

void CalculateSkyExposure(TH1D *hTau)
{

    Double_t dLogEnergyStep = 0.2; //0.2
    Double_t dHalfEnergyBinWidth =1/2.; //in log was 1/2
    Double_t logEmin = 7; //7
    Double_t logEmax = 11; //11

    bCombined = kTRUE;
    yMin = 5; //5
    yMax = 500; //500
    yDelta = 5; //5
    MaxElevation = 10; //elevation angle (determines path through Earth;
    //~ DeltaAngle = 0.05; //steps in azimuth and elevation 
    DeltaAngle = 0.1; //steps in azimuth and elevation 

    iConfig = 2; //telescope altitude
  
    //exposure
    Double_t dExposure=3*365*24*3600*0.20; //exposure time 3 years in seconds with 20% duty cycle

    Double_t dFoV = 2;  //test 0, 1, 2, 10
    tanFoV = tan(dFoV/180.*pi);
    //dFoVBelow = asin(REarth/(REarth+DetectorAltitude[iConfig]));
    dFoVBelow =  3/180.*pi; 
    iMirrorSize = 2;
    dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 

    dMinLength = 0.3; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.
    TGraph *grDiffAcceptance = new TGraph();

    TGraph *grSensitivity = new TGraph();


    TCanvas *cDiffSensitivity = new TCanvas("cDiffSensitivity","Differential Sensitivity",750,500);
    cDiffSensitivity->Draw();
    cDiffSensitivity->SetLogy();
    cDiffSensitivity->SetLogx();

    TGraph *grAcceptance = new TGraph();
    TCanvas *cAcceptance = new TCanvas("cAcceptance","Acceptance",750,500);
    cAcceptance->Draw();
    cAcceptance->SetLogy();
    cAcceptance->SetLogx();
  
    TCanvas *cTriggeredAzimuthAngles = new TCanvas("cTriggeredAzimuthAngles","Triggered Azimuth Angles",750,500);
    hTriggeredAzimuthAngles->Draw("HIST");

  


    //Move in steps from lowest to highes energy
    Double_t dLogE = logEmin;
    Int_t n=0;
    while(dLogE<=logEmax)
       {
          Double_t dAcceptance = CalculateAcceptance(dLogE-dHalfEnergyBinWidth,dLogE+dHalfEnergyBinWidth,grDiffAcceptance,hTau);
          //~ Double_t dAcceptance = CalculateAcceptance2(7,9.0000001,grDiffAcceptance,hTau);
    


          if(dAcceptance>1)
           {
              //Double_t dEIndexed = pow(10,-dLogE*nuIndex);

              Double_t dnuFnu = 3 * pow(10,dLogE*2) / dAcceptance / dExposure / (pow(10,dLogE+dHalfEnergyBinWidth)- pow(10,dLogE-dHalfEnergyBinWidth));
              cout<<"Energy "<<dLogE-dHalfEnergyBinWidth<<" to "<<dLogE+dHalfEnergyBinWidth<<" acceptance:  "<<dAcceptance<<" nuFnu: "<<dnuFnu<<" for power law with index -"<<nuIndex<<endl;
              grSensitivity->SetPoint(n,pow(10,dLogE),dnuFnu);
              cDiffSensitivity->cd();
              grSensitivity->Draw("alp");
              cDiffSensitivity->Modified();
              cDiffSensitivity->Update();

              grAcceptance->SetPoint(n,pow(10,dLogE),dAcceptance);
              cAcceptance->cd();
              grAcceptance->Draw("alp");
              cAcceptance->Modified();
              cAcceptance->Update();

              cTriggeredAzimuthAngles->cd();
              cTriggeredAzimuthAngles->Modified();
              cTriggeredAzimuthAngles->Update();

              n++;
            }
          dLogE+=dLogEnergyStep;
          break;
       }

  grSensitivity->SetLineWidth(3);
  grSensitivity->SetLineColor(kBlue+3);
  grSensitivity->GetXaxis()->SetTitle("energy [GeV]");
  grSensitivity->GetYaxis()->SetTitle("E^{2} dN/dE [ GeV cm^{-2} s^{-1} sr^{-1} ]");
  grSensitivity->GetYaxis()->SetTitleSize(0.04);
  grSensitivity->GetYaxis()->SetLabelSize(0.04);
  grSensitivity->GetXaxis()->SetTitleSize(0.04);
  grSensitivity->GetXaxis()->SetLabelSize(0.04);

  grAcceptance->SetLineWidth(3);
  grAcceptance->SetLineColor(kBlue+3);
  grAcceptance->GetXaxis()->SetTitle("energy [GeV]");
  grAcceptance->GetYaxis()->SetTitle("acceptance [ cm^{2} sr ]");
  grAcceptance->GetYaxis()->SetTitleSize(0.04);
  grAcceptance->GetYaxis()->SetLabelSize(0.04);
  grAcceptance->GetXaxis()->SetTitleSize(0.04);
  grAcceptance->GetXaxis()->SetLabelSize(0.04);

  hTriggeredAzimuthAngles->SetLineWidth(3);
  hTriggeredAzimuthAngles->SetLineColor(kBlue+3);
  hTriggeredAzimuthAngles->GetYaxis()->SetTitleSize(0.04);
  hTriggeredAzimuthAngles->GetYaxis()->SetLabelSize(0.04);
  hTriggeredAzimuthAngles->GetXaxis()->SetTitleSize(0.04);
  hTriggeredAzimuthAngles->GetXaxis()->SetLabelSize(0.04);
  hTriggeredAzimuthAngles->Scale(1.0/hTriggeredAzimuthAngles->Integral(),"nosw2");

  TString Filename;
  Filename.Form("SensitivityResults/new4/DifferentialSensitivityTrinity_10TimesNSB_%ikmAboveGround_%0.0fsqrmMirror_%0.1fdegUpperFoV_%0.1fdegLowerFoV_%0.1fdegMinShowerLength.root",iConfig,dMirrorA[iMirrorSize],dFoV,dFoVBelow/pi*180.,dMinLength);
  cDiffSensitivity->SaveAs(Filename.Data());
  Filename.Form("SensitivityResults/new4/AcceptanceTrinity_10TimesNSB_%ikmAboveGround_%0.0fsqrmMirror_%0.1fdegUpperFoV_%0.1fdegLowerFoV_%0.1fdegMinShowerLength.root",iConfig,dMirrorA[iMirrorSize],dFoV,dFoVBelow/pi*180.,dMinLength);
  cAcceptance->SaveAs(Filename.Data());
  Filename.Form("SensitivityResults/new4/TriggeredAzimuthTrinity_10TimesNSB_%ikmAboveGround_%0.0fsqrmMirror_%0.1fdegUpperFoV_%0.1fdegLowerFoV_%0.1fdegMinShowerLength.root",iConfig,dMirrorA[iMirrorSize],dFoV,dFoVBelow/pi*180.,dMinLength);
  cTriggeredAzimuthAngles->SaveAs(Filename.Data());
}

void GetAcceptanceSingleAngle(Double_t dMinEnu, Double_t dMaxEnu, TH1D *hTau, TH2F *skymapSingleAngle1)
{	
	//set the proper values for the energy
	dMinEnu = pow(10,dMinEnu);
	dMaxEnu = pow(10,dMaxEnu);
	
	Double_t dDeltaTelescopeAzimuth = DeltaAngleAz;
	Double_t dConversion = yDelta*dDeltaTelescopeAzimuth*pi/180.0; //multiply area of cell taking into account that we have a 360 degree FoV
	dConversion *= 1e10; //from km2 to cm2
	//~ dConversion*=DeltaAngleAz/180.*pi*DeltaAngle/180.*pi;
	Double_t dDeltaAcceptance = 0;
	Double_t dP = 0;
	Double_t dPFluorescence = 0.0;
	Double_t dPCherenkov = 0.0;
	Double_t elevation = 0.; 
	Double_t azimuth = 0.;
	Double_t dEarth;
	Double_t y = yMin;
	
	while(y < yMax) //looping over distance from telescope w/ incrememnts of yDelta
	{	
		if( bFluorescence || (bCombined && y<dMaxFluorescenceDistance) )
			MaxAzimuth = 180.0;
		for(int elv = 0; elv <= (int)(MaxElevation / DeltaAngle); elv++) //looping over elevation w/ steps of DeltaAngle
		{
			elevation = elv * DeltaAngle;
			
			for(int azi = 0; azi <= (int)(MaxAzimuth / DeltaAngleAz); azi++) //looping over azimuth w/ steps of DeltaAngleAz
			{
				azimuth = azi * DeltaAngleAz;
				dEarth = DistanceThroughEarth(y, elevation, azimuth);
				GetTauDistribution(hTau,dEarth,dMinEnu,dMaxEnu); //tau distribution is calculated
				dDeltaAcceptance = 0;
				dP = 0;
				for(int i=0;i<hTau->GetNbinsX();i++)
				{
					if(hTau->GetBinContent(i+1)>0)
					{
						dPFluorescence = 0.0;
						dPCherenkov = 0.0;
						
						if( bFluorescence || (bCombined && y<dMaxFluorescenceDistance) )
							dPFluorescence = PDecayFluorescence(hTau->GetBinCenter(i+1),y,elevation,azimuth);
						if( (!bFluorescence || bCombined) && azimuth<dMaxCherenkovAzimuthAngle  )
							dPCherenkov = PDecay(hTau->GetBinCenter(i+1),y,elevation,azimuth);
							
						if(bCombined)
							dP = dPFluorescence > dPCherenkov ? dPFluorescence : dPCherenkov;
						else if(bFluorescence)
							dP = dPFluorescence;
						else
							dP = dPCherenkov;
							
						dDeltaAcceptance+=hTau->GetBinContent(i+1)*dP;
					}
				}
				//the acceptances are loaded into the histogram with conversion factors applied
				if(azimuth != 0.0)
					skymapSingleAngle1->Fill(azimuth, (-1 * elevation), dDeltaAcceptance*sin(elevation/180.*pi)*y*dConversion); 
				skymapSingleAngle1->Fill((-1 * azimuth), (-1 * elevation), dDeltaAcceptance*sin(elevation/180.*pi)*y*dConversion);
				//~ cout<<dDeltaAcceptance*sin(elevation/180.*pi)*y*dConversion<<endl;
			}
		}
		y += yDelta; //distance from telescope counter increased by yDelta
	}
}

void PlotAcceptanceVsEnergy(TH1D *hTau)
{
	yMin = 20; //min distance from telescope where tau comes out of the ground in km
	yMax = 26;
	DeltaAngleAz = 0.1; //azimuth angle step
	DeltaAngle = 0.1; //elevation angle step
	MaxAzimuth = 180.; //max azi angle
	MaxElevation = 40; //max elv angle 
	bCombined = kTRUE; //both flor and cher events considered
    
    //values from differential sensitivity calculations
    yDelta = 5.0; //5
    iConfig = 2; //telescope altitude
    Double_t dFoV = 2;  //test 0, 1, 2, 10
    tanFoV = tan(dFoV/180.*pi);
    dFoVBelow =  3/180.*pi; 
    iMirrorSize = 2;
    dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 
    dMinLength = 0.3;
    
    Double_t logEmin = 6.0; //min energy log
    Double_t logEmax = 10.0; //max energy log
    Double_t logE = logEmin; //neutrino energy of interest log
    Double_t logEstep = 0.1; //step in energy log
    int nSteps = (int)ceil((logEmax - logEmin) / logEstep + 1);
    Double_t eng[nSteps], acc[nSteps];
    
    TH2F *skymapSingleAngle = new TH2F("skymapSingleAngle1","Acceptance Skymap of Single Azimuth Angle [20 km to 150 km, 10^9 GeV]", 3601, -180.05, 180.05, 1801, -90.05, 90.05); //histo for single angle acceptance plot
    TH2F *skymapFull360Sweep = new TH2F("skymapFull360Sweep1","Acceptance Skymap of 360 Degree Airshower Azimuth Sweep [20 km to 150 km, 10^9 GeV]", 3601, -180.05, 180.05, 1801, -90.05, 90.05);
    
    for(int i = 0; i < nSteps; i++) {
		skymapSingleAngle->Reset("ICESM");
		skymapFull360Sweep->Reset("ICESM");
		
		GetAcceptanceSingleAngle(logE, (logE + 0.1), hTau, skymapSingleAngle);
		
		for(int yBins = 1; yBins <= skymapSingleAngle->GetNbinsY(); yBins++)
		{
			Double_t comboBin = 0;
			for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
				comboBin += skymapSingleAngle->GetBinContent(xBins, yBins);
			for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
				skymapFull360Sweep->SetBinContent(xBins, yBins, comboBin);
		}
		
		Double_t totalAcc = 0;
		
		for(int i = 1; i <= skymapFull360Sweep->GetNbinsX(); i++)
		{
			for(int j = 1; j <= skymapFull360Sweep->GetNbinsY(); j++)
				totalAcc += skymapFull360Sweep->GetBinContent(i, j);
		}
		
		eng[i] = pow(10.0, logE);
		acc[i] = totalAcc;
		
		logE += 0.1;
	}
    
    TCanvas *accVE = new TCanvas("accVE", "Acceptance v. Neutrino Energy", 1300, 900);
    TGraph *accVSEplot = new TGraph(nSteps, eng, acc);
    accVSEplot->SetLineColor(2);
	accVSEplot->SetLineWidth(4);
	accVSEplot->SetTitle("Plot of Acceptance Vs. Neutino Energy");
	accVSEplot->GetYaxis()->SetTitle("Acceptance [cm^{2}]");
	accVSEplot->GetXaxis()->SetTitle("Neutrino Energy [GeV]");
	accVE->SetLogy(1);
	accVE->SetLogx(1);
	accVE->SetGridx(1);
	accVE->SetGridy(1);
	accVSEplot->Draw("AC");
}

void PlotAcceptanceSkymaps(TH1D *hTau)
{
	latitude = 38.52028; //lat of frisco peak, utah
	tStep = 2.5; //10 min step in degrees
	yMin = 20; //min distance from telescope where tau comes out of the ground in km
	yMax = 26;
	DeltaAngleAz = 0.1; //azimuth angle step
	DeltaAngle = 0.1; //elevation angle step
	MaxAzimuth = 180.; //max azi angle
	MaxElevation = 40; //max elv angle 
	bCombined = kTRUE; //both flor and cher events considered
	Double_t logEmin = 6.0; //min energy log
    Double_t logEmax = 10.0; //max energy log
    Double_t LST = 0;
	Double_t degconv = pi/180.0;
	Double_t Enaught = 100000; //GeV from IceCube paper (100 TeV)
	Double_t Fnaught = 1.6e-18; //TeV^-1 cm^-2 s^-1 flux normalization at 100 TeV from IceCube paper over ~158 day period
	Double_t normInverse = (pow(pow(10, logEmin), (1 - nuIndex)) - pow(pow(10, logEmax), (1 - nuIndex))) / (nuIndex - 1); // integral of E^-nuIndex from Emin to Emax to correct for the normalization in the GetTauDistibution function
	//~ normInverse = 1.0;
	//~ multNorm = kFALSE;
	
	//values from differential sensitivity calculations
    yDelta = 5.0; //5
    iConfig = 2; //telescope altitude
    Double_t dFoV = 2;  //test 0, 1, 2, 10
    tanFoV = tan(dFoV/180.*pi);
    dFoVBelow =  3/180.*pi; 
    iMirrorSize = 2;
    dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 
    dMinLength = 0.3;
	
	//new canvas for the horizontal skymaps, markers and labels for galactic landmarks
	TCanvas *skyC = new TCanvas("skyC","Skymap of Acceptance",1600,750); 
	TMarker *galMarks[10];
	TMarker *galMarksSup[10];
	TMarker *galMarksEq[10];
	TMarker *galMarksEqI[10];
	TText *labels[10];
	TText *labelsSup[10];
	TText *labelsEq[10];
	TText *labelsEqI[10];
	TPad *pad1 = new TPad("pad1","",0,0,1,1);
	
	//marker initialization
	for(int i = 0; i < 10; i++) {
		galMarks[i] = new TMarker(0.,0., 43);
		galMarksSup[i] = new TMarker(0.,0., 43);
		galMarksEq[i] = new TMarker(0.,0., 43);
		galMarksEqI[i] = new TMarker(0.,0., 43);
		galMarks[i]->SetMarkerSize(1.5);
		galMarksSup[i]->SetMarkerSize(1.5);
		galMarksEq[i]->SetMarkerSize(1.5);
		galMarksEqI[i]->SetMarkerSize(1.5);
	}

	//label initialization
	for(int i = 0; i < 10; i++) {
		labels[i] = new TText();
		labels[i]->SetTextSize(20);
		labels[i]->SetTextFont(43);
		labels[i]->SetTextAlign(22);
		labelsSup[i] = new TText();
		labelsSup[i]->SetTextSize(20);
		labelsSup[i]->SetTextFont(43);
		labelsSup[i]->SetTextAlign(22);
		labelsEq[i] = new TText();
		labelsEq[i]->SetTextSize(20);
		labelsEq[i]->SetTextFont(43);
		labelsEq[i]->SetTextAlign(22);
		labelsEqI[i] = new TText();
		labelsEqI[i]->SetTextSize(20);
		labelsEqI[i]->SetTextFont(43);
		labelsEqI[i]->SetTextAlign(22);
	}
	
	//pad init
	pad1->SetFillStyle(4000);
	pad1->SetFillColor(0);
	pad1->SetBorderSize(0);
	pad1->SetFrameBorderMode(0);
	pad1->SetFrameLineColor(0); 
	pad1->SetFrameBorderMode(0);
	pad1->Range(-231,-111.875,283,111.875);
	
	//calculations done for the gridlines of each skymap
	float conv=TMath::Pi()/180; 
	float la, lo, x, yy, z;
	int Nl = 5; // Number of drawn latitudes
	int NL = 5; // Number of drawn longitudes
	int M  = 30;
	
	TGraph  *latitudes[Nl];
	TGraph  *longitudes[NL];
	
	for (int j=0;j<Nl;++j) {
		latitudes[j]=new TGraph();
		la = -90+180/(Nl-1)*j;
		for (int i=0;i<M+1;++i) {
			lo = -180+360/M*i;
			z  = sqrt(1+cos(la*conv)*cos(lo*conv/2));
			x  = 180*cos(la*conv)*sin(lo*conv/2)/z;
			yy  = 90*sin(la*conv)/z;
			latitudes[j]->SetPoint(i,x,yy);
		}
	}
	
	for (int j=0;j<NL;++j) {
		longitudes[j]=new TGraph();
		lo = -180+360/(NL-1)*j;
		for (int i=0;i<M+1;++i) {
			la = -90+180/M*i;
			z  = sqrt(1+cos(la*conv)*cos(lo*conv/2));
			x  = 180*cos(la*conv)*sin(lo*conv/2)/z;
			yy  = 90*sin(la*conv)/z;
			longitudes[j]->SetPoint(i,x,yy);
		}
	}
	
	//2D histograms for various skymaps + histogram to store number of neutrino events
	TH2F *skymapSingleAngle = new TH2F("skymapSingleAngle","Acceptance Skymap of Single Azimuth Angle", 3601, -180.05, 180.05, 1801, -90.05, 90.05); //histo for single angle acceptance plot
	TH2F *skymapFull360Sweep = new TH2F("skymapFull360Sweep","Acceptance Skymap of 360 Degree Airshower Azimuth Sweep", 3601, -180.05, 180.05, 1801, -90.05, 90.05);
	TH2F *skymapFullProjection = new TH2F("skymapFullProjection","360 FoV Projection In Galactic Coordinates Over MJD 56937.81 to MJD 57096.21", 361, -180.05, 180.05, 181, -90.05, 90.05); //galactic
	TH2F *skymapProjSuperGal = new TH2F("skymapProjSuperGal","360 FoV Projection In Supergalactic Coordinates Over MJD 56937.81 to MJD 57096.21", 361, -180.05, 180.05, 181, -90.05, 90.05); //supergal
	TH2F *skymapProjEq = new TH2F("skymapProjEq","360 FoV Projection In Equatorial Coordinates Over MJD 56937.81 to MJD 57096.21", 361, -180.05, 180.05, 181, -90.05, 90.05); //equatorial
	TH2F *skymapInstantConverage = new TH2F("skymapInstantConverage","Instantanious Sky Coverage In Equatorial Coordinates", 361, -180.05, 180.05, 181, -90.05, 90.05); //histogram for instant sky coverage
	TH2F *nuevents = (TH2F*)skymapProjEq->Clone("nuevents");
	
	//histogram formatting
	skymapSingleAngle->GetXaxis()->SetTitle("Azimuth Angle [degrees]");
    skymapSingleAngle->GetYaxis()->SetTitle("Elevation Angle [degrees]");
    skymapFull360Sweep->GetXaxis()->SetTitle("Azimuth Angle [degrees]");
    skymapFull360Sweep->GetYaxis()->SetTitle("Elevation Angle [degrees]");
    skyC->Divide(2,1);
    
    //getting the single angle acceptance according to the variables above 
    GetAcceptanceSingleAngle(logEmin, logEmax, hTau, skymapSingleAngle);
    
    
    //projecting the single angle of acceptance over a 360 degree FoV
    for(int yBins = 1; yBins <= skymapSingleAngle->GetNbinsY(); yBins++)
    {
		Double_t comboBin = 0;
		for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
			comboBin += skymapSingleAngle->GetBinContent(xBins, yBins);
		for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
			skymapFull360Sweep->SetBinContent(xBins, yBins, comboBin);
	}
	
	skyC->cd(1);
	gPad->SetLogz(1);
	skymapSingleAngle->Draw("COLZ"); //plot single angle skymap
	
	skyC->cd(2);
	gPad->SetLogz(1);
	skymapFull360Sweep->Draw("COLZ"); //plot 360 sweep skymap
	ifstream in;
	in.open("1yr.txt"); //open ephem file
	//~ return;
	for(int r = -180; r <= 180; r++) { //filling the instant sky converage histogram
		for(int d = -90; d <= 90; d++) {
			Double_t az = (atan2(sin((LST - r) * degconv), cos((LST - r) * degconv) * sin(latitude * degconv) - tan(d * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
			//~ Double_t az = (-1.0 * atan2(sin((LST - r) * degconv) * cos(d * degconv), cos(latitude * degconv) * sin(d * degconv) - sin(latitude * degconv) * cos(d * degconv) * cos((LST - r) * degconv)) * 180 / pi) - 180;
			Double_t alt = asin(sin(latitude * degconv) * sin(d * degconv) + cos(latitude * degconv) * cos(d * degconv) * cos((LST - r) * degconv)) * 180 / pi;
			if(az > 180.0)
				az = az - 360.0;
			else if(az < -180.0)
				az = az + 360.0;
			//~ Double_t dec = asin(sin(d * degconv) * sin(27.1284 * degconv) + cos(d * degconv) * cos(27.1284 * degconv) * cos((122.9320 - r) * degconv)) * 180 / pi;
			//~ Double_t ra = (atan2((cos(d * degconv) * sin((122.932 - r) * degconv)), (sin(d * degconv) * cos(27.1284 * degconv) - cos(d * degconv) * sin(27.1284 * degconv) * cos((122.9320 - r) * degconv))) * 180 / pi) + 192.8595;
			//~ if(ra > 180)
				//~ ra = ra - 360.;
			//~ if(ra < -180)
				//~ ra = ra + 360.;
			//~ Double_t az = (atan2(sin((LST - ra) * degconv), cos((LST - ra) * degconv) * sin(latitude * degconv) - tan(dec * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
			//~ Double_t alt = asin(sin(latitude * degconv) * sin(dec * degconv) + cos(latitude * degconv) * cos(dec * degconv) * cos((LST - ra) * degconv)) * 180 / pi;
			//~ if(az > 180.0)
				//~ az = az - 360.0;
			//~ if(az < -180.0)
				//~ az = az + 360.0;
			int xBin = (int)((az + 180.1) * 10);
			int yBin = (int)((alt + 90.1) * 10);
			skymapInstantConverage->Fill((-1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin));
		}
	}
	
	TCanvas *skyProjInstantEq = new TCanvas("skyProjInstantEq","Instant Skymap Coverage (Equatorial Coordinates)",1500,750); //canvas for instant converage skymap
	TPad *padI = (TPad*)pad1->Clone("padI");
	
	skyProjInstantEq->cd(1);
	skyProjInstantEq->SetRightMargin(0.2);
	
	skymapInstantConverage->GetXaxis()->SetTitle("Right Ascension [deg]");
	skymapInstantConverage->GetYaxis()->SetTitle("Declination [deg]");
	skymapInstantConverage->GetZaxis()->SetTitle("Acceptance [cm^{2}]");
	skymapInstantConverage->Draw("z aitoff");
	padI->Draw();
	
	galMarksEqI[0]->SetX(84.7649); //galactic center
	galMarksEqI[0]->SetY(-32.1444);
	
	galMarksEqI[1]->SetX(-77.0868); //TXS
	galMarksEqI[1]->SetY(6.1484);
	
	galMarksEqI[2]->SetX(86.8957); //MRK 501
	galMarksEqI[2]->SetY(45.1052);
	
	galMarksEqI[3]->SetX(-132.496); //MRK 421
	galMarksEqI[3]->SetY(52.534);
	
	galMarksEqI[4]->SetX(161.754); //Virgo
	galMarksEqI[4]->SetY(-9.72109);
	
	galMarksEqI[5]->SetX(49.3004); //cygnus A
	galMarksEqI[5]->SetY(42.3715);
	
	galMarksEqI[6]->SetX(119.23); // Cen A
	galMarksEqI[6]->SetY(-56.6101);
	
	galMarksEqI[7]->SetX(-92.1939); //Auger Dipole
	galMarksEqI[7]->SetY(-29.1072);
	
	galMarksEqI[8]->SetX(-44.1018); //Fornax
	galMarksEqI[8]->SetY(-34.7217);
	
	galMarksEqI[9]->SetX(-102.613); //TA Hotspot
	galMarksEqI[9]->SetY(52.2853);
	
	//~ galMarksEqI[0]->SetX(0); //galactic center
	//~ galMarksEqI[0]->SetY(0);
	
	//~ galMarksEqI[1]->SetX(155.711); //TXS
	//~ galMarksEqI[1]->SetY(-28.0311);
	
	//~ galMarksEqI[2]->SetX(-53.1613); //MRK 501
	//~ galMarksEqI[2]->SetY(40.6421);
	
	//~ galMarksEqI[3]->SetX(-75.9516); //MRK 421
	//~ galMarksEqI[3]->SetY(81.5564);
	
	//~ galMarksEqI[4]->SetX(32.3546); //Virgo
	//~ galMarksEqI[4]->SetY(57.0701);
	
	//~ galMarksEqI[5]->SetX(-75.9174); //cygnus A
	//~ galMarksEqI[5]->SetY(6.20106);
	
	//~ galMarksEqI[6]->SetX(48.4862); // Cen A
	//~ galMarksEqI[6]->SetY(20.0392);
	
	//~ galMarksEqI[7]->SetX(122.036); //Auger Dipole
	//~ galMarksEqI[7]->SetY(-17.1514);
	
	//~ galMarksEqI[8]->SetX(75.2931); //Fornax
	//~ galMarksEqI[8]->SetY(-65.8552);
	
	//~ galMarksEqI[9]->SetX(-136.706); //TA Hotspot
	//~ galMarksEqI[9]->SetY(57.3636);
	
	labelsEqI[0]->SetText(galMarksEqI[0]->GetX() + 12, galMarksEqI[0]->GetY() -5, "Galactic Center");
	labelsEqI[1]->SetText(galMarksEqI[1]->GetX(), galMarksEqI[1]->GetY() -5, "TXS 0506+056");
	labelsEqI[2]->SetText(galMarksEqI[2]->GetX(), galMarksEqI[2]->GetY() -5, "MRK 501");
	labelsEqI[3]->SetText(galMarksEqI[3]->GetX() - 15, galMarksEqI[3]->GetY(), "MRK 421");
	labelsEqI[4]->SetText(galMarksEqI[4]->GetX(), galMarksEqI[4]->GetY() -5, "Virgo");
	labelsEqI[5]->SetText(galMarksEqI[5]->GetX(), galMarksEqI[5]->GetY() -5, "Cygnus A");
	labelsEqI[6]->SetText(galMarksEqI[6]->GetX(), galMarksEqI[6]->GetY() -5, "Cen A");
	labelsEqI[7]->SetText(galMarksEqI[7]->GetX() - 7, galMarksEqI[7]->GetY() -5, "Auger Dipole");
	labelsEqI[8]->SetText(galMarksEqI[8]->GetX(), galMarksEqI[8]->GetY() -5, "Fornax");
	labelsEqI[9]->SetText(galMarksEqI[9]->GetX() + 9, galMarksEqI[9]->GetY() -5, "TA Hotspot");
	
	padI->cd();
	
	for (int j=0;j<Nl;++j) latitudes[j]->Draw("l");
	for (int j=0;j<NL;++j) longitudes[j]->Draw("l");
	
	for(int i = 0; i < 10; i++) {
		galMarksEqI[i]->Draw();
		labelsEqI[i]->Draw();
	}
	
	gStyle->SetPalette(56);
	//~ return;//testing
	
	Double_t totalT = 0, totalAcc = 0, gammarayAcc = 0;
	//calculations for the time evolution of the horizontal skymaps over various coordinate systems
	if (in.is_open())
	{
		Double_t setTimeSun, riseTimeSun, riseTimeMoon, setTimeMoon, phaseMoon, deltaT = 0, tStepAdjusted, totAccDay = 0; 
		//~ Double_t origLST;
		string sTimeS, rTimeS, rTimeM, sTimeM, phM;
		bool nestNone = false, nestSun = false, nestBoth = false;
		int nSteps;
		
		while(in.good()) 
		{
			in >> sTimeS >> rTimeS >> rTimeM >> sTimeM >> phM; //read from the file that contains rise and set times of moon and sun in LST as well as phase of moon
			
			setTimeSun = stod(sTimeS);
			riseTimeSun = stod(rTimeS);
			riseTimeMoon = stod(rTimeM);
			setTimeMoon = stod(sTimeM);
			phaseMoon = stod(phM);
			
			nestNone = false;
			nestSun = false;
			nestBoth = false;
			
			totAccDay = 0;
			
			//moon phase calculations
			if(phaseMoon < 0.3) {
				if(setTimeSun > riseTimeSun)
					deltaT = (360.0 - setTimeSun) + riseTimeSun;
				else
					deltaT = riseTimeSun - setTimeSun;
				LST = setTimeSun;
			} else {
				if( (riseTimeSun > setTimeSun) && (setTimeMoon > riseTimeMoon) ) { //both don't cross 0hr
					if( (riseTimeMoon < setTimeSun) && (setTimeMoon < setTimeSun) && (setTimeMoon < riseTimeSun) ){
						LST = setTimeMoon;
						deltaT = riseTimeSun - setTimeMoon;
					} else if( (riseTimeMoon > setTimeSun) && (setTimeMoon > riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeMoon - setTimeSun;
					} else if( (riseTimeMoon > setTimeSun) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (riseTimeMoon - setTimeSun) + (riseTimeSun - setTimeMoon);
						nestNone = true;
					} else if( (riseTimeMoon < setTimeSun && setTimeMoon < setTimeSun) || (riseTimeMoon > riseTimeSun && setTimeMoon > riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeSun - setTimeSun;
					} else { deltaT = 0; }
				} else if( (riseTimeSun < setTimeSun) && (setTimeMoon > riseTimeMoon) ) { //only sun crosses 0hr (impossible for moon to cover full night)
					if( (riseTimeMoon < setTimeSun) && (setTimeMoon > setTimeSun) ) {
						LST = setTimeMoon;
						deltaT = (360. + riseTimeSun) - setTimeMoon;
					} else if( (riseTimeSun > riseTimeMoon) && (riseTimeSun < setTimeMoon) ) {
						LST = setTimeSun;
						deltaT = (360. + riseTimeMoon) - setTimeSun;
					} else if( (riseTimeMoon > setTimeSun && setTimeMoon < 360.) || (riseTimeMoon < riseTimeSun && setTimeMoon < riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (riseTimeSun + 360. - setTimeSun) - (setTimeMoon - riseTimeMoon);
						nestSun = true;
					} else if( (riseTimeMoon < setTimeSun && setTimeMoon < setTimeSun) || (riseTimeMoon > riseTimeSun && setTimeMoon > riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (360. + riseTimeSun) - setTimeSun;
					}
				} else if( (riseTimeSun > setTimeSun) && (setTimeMoon < riseTimeMoon) ) { //only moon crosses 0hr
					if( ((setTimeSun + 360.) > riseTimeMoon) && (setTimeMoon > setTimeSun) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeMoon;
						deltaT = riseTimeSun - setTimeMoon;
					} else if( (riseTimeMoon < riseTimeSun) && (riseTimeSun < (setTimeMoon + 360.)) && (riseTimeMoon > setTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeMoon - setTimeSun;
					} else if( (riseTimeMoon > riseTimeSun) && (setTimeMoon < setTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeSun - setTimeSun;
					} else { deltaT = 0; }
				} else if ( (riseTimeSun < setTimeSun)  && (setTimeMoon < riseTimeMoon) ) { //both cross 0 hr
					if( (riseTimeMoon > setTimeSun) && (riseTimeSun < setTimeMoon) ) {
						LST = setTimeSun;
						deltaT = riseTimeMoon - setTimeSun;
					} else if( (riseTimeMoon < setTimeSun) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeMoon;
						deltaT = riseTimeSun - setTimeMoon;
					} else if( (setTimeSun < riseTimeMoon) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (riseTimeSun + 360. - setTimeSun) - (setTimeMoon + 360. - riseTimeMoon);
						nestBoth = true;
					} else { deltaT = 0; }
				}
			}
			
			totalT += deltaT;
			nSteps = (int)(deltaT / tStep);
			tStepAdjusted = deltaT / nSteps;
			//~ origLST = LST;
			/*
			for(int i = 0; i < nSteps; i++) { //galactic
				for(int r = -180; r <= 180; r++) {
					for(int d = -90; d <= 90; d++) {
						Double_t dec = asin(sin(d * degconv) * sin(27.1284 * degconv) + cos(d * degconv) * cos(27.1284 * degconv) * cos((122.9320 - r) * degconv)) * 180 / pi;
						Double_t ra = (atan2((cos(d * degconv) * sin((122.932 - r) * degconv)), (sin(d * degconv) * cos(27.1284 * degconv) - cos(d * degconv) * sin(27.1284 * degconv) * cos((122.9320 - r) * degconv))) * 180 / pi) + 192.8595;
						if(ra > 180)
							ra = ra - 360.;
						if(ra < -180)
							ra = ra + 360.;
						Double_t az = (atan2(sin((LST - ra) * degconv), cos((LST - ra) * degconv) * sin(latitude * degconv) - tan(dec * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
						Double_t alt = asin(sin(latitude * degconv) * sin(dec * degconv) + cos(latitude * degconv) * cos(dec * degconv) * cos((LST - ra) * degconv)) * 180 / pi;
						if(az > 180.0)
							az = az - 360.0;
						if(az < -180.0)
							az = az + 360.0;
						int xBin = (int)((az + 180.1) * 10);
						int yBin = (int)((alt + 90.1) * 10);
						if( !( LST > riseTimeMoon && LST < setTimeMoon) && 
							!( LST > riseTimeMoon && LST < setTimeMoon) && 
							!( ( (LST > riseTimeMoon && LST < 360.) || (LST > 0 && LST < setTimeMoon) )) ) 
							{ 
								skymapFullProjection->Fill((-1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin) * tStepAdjusted * 240); //240 sec = 1 degree of RA
							} 
							//~ { skymapFullProjection->Fill((1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin)); }
					}
				}
				LST += tStepAdjusted;
				if(LST > 360.0)
					LST -= 360.0;
			}
			
			LST = origLST;
			
			for(int i = 0; i < nSteps; i++) { //supergalactic
				for(int r = -180; r <= 180; r++) {
					for(int d = -90; d <= 90; d++) {
						Double_t dec = asin(sin(d * degconv) * sin(15.70894274 * degconv) + cos(d * degconv) * cos(15.70894274 * degconv) * cos((26.45043911 - r) * degconv)) * 180 / pi;
						Double_t ra = (atan2((cos(d * degconv) * sin((26.45043911 - r) * degconv)), (sin(d * degconv) * cos(15.70894274 * degconv) - cos(d * degconv) * sin(15.70894274 * degconv) * cos((26.45043911 - r) * degconv))) * 180 / pi) + 283.75418652;
						if(ra > 180)
							ra = ra - 360.;
						if(ra < -180)
							ra = ra + 360.;
						Double_t az = (atan2(sin((LST - ra) * degconv), cos((LST - ra) * degconv) * sin(latitude * degconv) - tan(dec * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
						Double_t alt = asin(sin(latitude * degconv) * sin(dec * degconv) + cos(latitude * degconv) * cos(dec * degconv) * cos((LST - ra) * degconv)) * 180 / pi;
						if(az > 180.0)
							az = az - 360.0;
						if(az < -180.0)
							az = az + 360.0;
						int xBin = (int)((az + 180.1) * 10);
						int yBin = (int)((alt + 90.1) * 10);
						if( !( LST > riseTimeMoon && LST < setTimeMoon) && 
							!( LST > riseTimeMoon && LST < setTimeMoon) && 
							!( ( (LST > riseTimeMoon && LST < 360.) || (LST > 0 && LST < setTimeMoon) )) ) 
							{ 
								skymapProjSuperGal->Fill((-1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin) * tStepAdjusted * 240); //240 sec = 1 degree of RA
							} 
					}
				}
				LST += tStepAdjusted;
				if(LST > 360.0)
					LST -= 360.0;
			}
			*/
			//~ LST = origLST;
			//evolving the horizotal skymap over time and projecting onto equatorial coordinates
			for(int i = 0; i < nSteps; i++) { //eqatorial
				for(int r = -180; r <= 180; r++) {
					for(int d = -90; d <= 90; d++) {
						Double_t az = (atan2(sin((LST - r) * degconv), cos((LST - r) * degconv) * sin(latitude * degconv) - tan(d * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
						Double_t alt = asin(sin(latitude * degconv) * sin(d * degconv) + cos(latitude * degconv) * cos(d * degconv) * cos((LST - r) * degconv)) * 180 / pi;
						if(az > 180.0)
							az = az - 360.0;
						else if(az < -180.0)
							az = az + 360.0;
						int xBin = (int)((az + 180.1) * 10);
						int yBin = (int)((alt + 90.1) * 10);
						if( !(nestNone && LST > riseTimeMoon && LST < setTimeMoon) && 
							!(nestSun && LST > riseTimeMoon && LST < setTimeMoon) && 
							!(nestBoth && ( (LST > riseTimeMoon && LST < 360.) || (LST > 0 && LST < setTimeMoon) ) ) ) 
							{ 
								skymapProjEq->Fill((-1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin) * tStepAdjusted * 240.0); //240 sec = 1 degree of RA
								totAccDay += skymapFull360Sweep->GetBinContent(xBin, yBin) * tStepAdjusted * 240.0;
								//~ nuevents->Fill((-1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin) * tStepAdjusted * 240 * normInverse * Fnaught / pow(Enaught, -nuIndex));
							} 
					}
				}
				LST += tStepAdjusted;
				if(LST > 360.0)
					LST -= 360.0;
			}
			gammarayAcc += totAccDay * (deltaT / 360.) / (4. * pi);
		}
		//galactic & supergal projections done w.r.t the equatorial plot after time evolution has finished
		for(int l = -180; l <= 180; l++)
		{
			for(int b = -90; b <= 90; b++)
			{
				Double_t dec = asin(sin(b * degconv) * sin(27.1284 * degconv) + cos(b * degconv) * cos(27.1284 * degconv) * cos((122.9320 - l) * degconv)) * 180 / pi;
				Double_t ra = (atan2((cos(b * degconv) * sin((122.932 - l) * degconv)), (sin(b * degconv) * cos(27.1284 * degconv) - cos(b * degconv) * sin(27.1284 * degconv) * cos((122.9320 - l) * degconv))) * 180 / pi) + 192.8595;
				if(ra > 180)
					ra = ra - 360.;
				else if(ra < -180)
					ra = ra + 360.;
				int xBin = (int)(ra + 181);
				int yBin = (int)(dec + 91);
				skymapFullProjection->Fill(-1.0 * l, b, skymapProjEq->GetBinContent(xBin, yBin));
				
				dec = asin(sin(b * degconv) * sin(15.70894274 * degconv) + cos(b * degconv) * cos(15.70894274 * degconv) * cos((26.45043911 - l) * degconv)) * 180 / pi;
				ra = (atan2((cos(b * degconv) * sin((26.45043911 - l) * degconv)), (sin(b * degconv) * cos(15.70894274 * degconv) - cos(b * degconv) * sin(15.70894274 * degconv) * cos((26.45043911 - l) * degconv))) * 180 / pi) + 283.75418652;
				if(ra > 180)
					ra = ra - 360.;
				else if(ra < -180)
					ra = ra + 360.;
				xBin = (int)(ra + 181);
				yBin = (int)(dec + 91);
				skymapProjSuperGal->Fill(-1.0 * l, b, skymapProjEq->GetBinContent(xBin, yBin));
			}
		}
		//applying the IceCube parameters to the time-evolved acceptance to get the number of neutrino events in each bin
		for(int i = 1; i <= skymapProjEq->GetNbinsX(); i++)
		{
			for(int j = 1; j <= skymapProjEq->GetNbinsY(); j++)
				nuevents->SetBinContent(i, j, skymapProjEq->GetBinContent(i, j) * normInverse * Fnaught / pow(Enaught, -nuIndex));
		}
		
		in.close();
	} else {cout << "Unable to open file" << endl; return; }
	
	for(int i = 1; i <= skymapProjEq->GetNbinsX(); i++) {
		for(int j = 1; j <= skymapProjEq->GetNbinsY(); j++)
			totalAcc += skymapProjEq->GetBinContent(i, j);
	}
	
	//printing observation time, acceptance, and duty cycle
	cout<<"Total observation time: "<<totalT * (24.0 / 360.0)<<" hours."<<endl;
	cout<<"Total Acceptance over "<<totalT * (24.0 / 360.0)<<" hours: "<<totalAcc<<endl;
	cout<<"Duty Cycle: "<<totalT * (1. / (360. * 365.)) * 100.<<" percent"<<endl; //1 year
	//~ cout<<"Duty Cycle: "<<totalT * (24.0 / 360.0) * (1/87600.0) * 100.<<" percent"<<endl; //10 years
	
	//initializing and formatting graphical elements for the galactic coordinate skymap
	TCanvas *skyProjC = new TCanvas("skyProjC","Skymap Projection (Galactic Coordinates)",1500,750); //canvas for galactic skymap projections
	
	//pad and histogram formatting
	
	TPad *pad2 = (TPad*)pad1->Clone("pad2");
	TPad *pad3 = (TPad*)pad1->Clone("pad3");
	
	skyProjC->cd(1);
	gStyle->SetPalette(56);
	skyProjC->SetRightMargin(0.2);
	skymapFullProjection->GetXaxis()->SetTitle("Galactic Longitude [deg]");
	skymapFullProjection->GetYaxis()->SetTitle("Galactic Latitude [deg]");
	skymapFullProjection->GetZaxis()->SetTitle("Acceptance [cm^{2} s]");
	skymapFullProjection->Draw("z aitoff");
	pad1->Draw();
	
	//~ labels[0]->SetText(-29, -5, "Galactic Center");
	//~ labels[1]->SetText(-55., 5.0, "Cygnus A");
	//~ labels[2]->SetText(-35., 35., "MRK 501");
	//~ labels[3]->SetText(-97., 85., "MRK 421");
	//~ labels[4]->SetText(-125., 45., "TA Hotspot");
	//~ labels[5]->SetText(36., 55, "Virgo");
	//~ labels[6]->SetText(48, 25, "Cen A");
	//~ labels[7]->SetText(150, -10, "Auger Dipole");
	//~ labels[8]->SetText(135, -35, "TXS 0506+056");
	//~ labels[9]->SetText(75, -72, "Fornax");
	
	galMarks[0]->SetX(0); //galactic center
	galMarks[0]->SetY(0);
	
	galMarks[1]->SetX(155.711); //TXS
	galMarks[1]->SetY(-28.0311);
	
	galMarks[2]->SetX(-53.1613); //MRK 501
	galMarks[2]->SetY(40.6421);
	
	galMarks[3]->SetX(-75.9516); //MRK 421
	galMarks[3]->SetY(81.5564);
	
	galMarks[4]->SetX(32.3546); //Virgo
	galMarks[4]->SetY(57.0701);
	
	galMarks[5]->SetX(-75.9174); //cygnus A
	galMarks[5]->SetY(6.20106);
	
	galMarks[6]->SetX(48.4862); // Cen A
	galMarks[6]->SetY(20.0392);
	
	galMarks[7]->SetX(122.036); //Auger Dipole
	galMarks[7]->SetY(-17.1514);
	
	galMarks[8]->SetX(75.2931); //Fornax
	galMarks[8]->SetY(-65.8552);
	
	galMarks[9]->SetX(-136.706); //TA Hotspot
	galMarks[9]->SetY(57.3636);
	
	labels[0]->SetText(galMarks[0]->GetX(), galMarks[0]->GetY() -5, "Galactic Center");
	labels[1]->SetText(galMarks[1]->GetX(), galMarks[1]->GetY() -5, "TXS 0506+056");
	labels[2]->SetText(galMarks[2]->GetX(), galMarks[2]->GetY() -5, "MRK 501");
	labels[3]->SetText(galMarks[3]->GetX(), galMarks[3]->GetY() -5, "MRK 421");
	labels[4]->SetText(galMarks[4]->GetX(), galMarks[4]->GetY() -5, "Virgo");
	labels[5]->SetText(galMarks[5]->GetX(), galMarks[5]->GetY() -5, "Cygnus A");
	labels[6]->SetText(galMarks[6]->GetX(), galMarks[6]->GetY() -5, "Cen A");
	labels[7]->SetText(galMarks[7]->GetX(), galMarks[7]->GetY() -5, "Auger Dipole");
	labels[8]->SetText(galMarks[8]->GetX(), galMarks[8]->GetY() -5, "Fornax");
	labels[9]->SetText(galMarks[9]->GetX(), galMarks[9]->GetY() -5, "TA Hotspot");
	
	pad1->cd();
	
	for (int j=0;j<Nl;++j) latitudes[j]->Draw("l");
	for (int j=0;j<NL;++j) longitudes[j]->Draw("l");
	
	for(int i = 0; i < 10; i++) {
		galMarks[i]->Draw();
		labels[i]->Draw();
	}
	
	//initializing and formatting graphical elements for the supergalactic coordinate skymap
	TCanvas *skyProjCSuper = new TCanvas("skyProjCSuper","Skymap Projection (Supergalactic Coordinates)",1500,750); //canvas for supergalactic skymap projections
	
	skyProjCSuper->cd(1);
	skyProjCSuper->SetRightMargin(0.2);
	skymapProjSuperGal->GetXaxis()->SetTitle("Supergalactic Longitude [deg]");
	skymapProjSuperGal->GetYaxis()->SetTitle("Supergalactic Latitude [deg]");
	skymapProjSuperGal->GetZaxis()->SetTitle("Acceptance [cm^{2} s]");
	skymapProjSuperGal->Draw("z aitoff");
	pad2->Draw();
	
	galMarksSup[0]->SetX(129.873); //galactic center
	galMarksSup[0]->SetY(59.1844);
	
	galMarksSup[1]->SetX(17.194); //TXS
	galMarksSup[1]->SetY(-56.5666);
	
	galMarksSup[2]->SetX(45.5916); //MRK 501
	galMarksSup[2]->SetY(56.6805);
	
	galMarksSup[3]->SetX(-70.6736); //MRK 421
	galMarksSup[3]->SetY(-11.2826);
	
	galMarksSup[4]->SetX(-123.878); //Virgo
	galMarksSup[4]->SetY(1.7941);
	
	galMarksSup[5]->SetX(-.259471); //cygnus A
	galMarksSup[5]->SetY(61.3381);
	
	galMarksSup[6]->SetX(-159.147); // Cen A
	galMarksSup[6]->SetY(-7.42725);
	
	galMarksSup[7]->SetX(29.4819); //Auger Dipole
	galMarksSup[7]->SetY(-84.528);
	
	galMarksSup[8]->SetX(78.3434); //Fornax
	galMarksSup[8]->SetY(-42.7981);
	
	galMarksSup[9]->SetX(-46.701); //TA Hotspot
	galMarksSup[9]->SetY(-25.9257);
	
	labelsSup[0]->SetText(galMarksSup[0]->GetX(), galMarksSup[0]->GetY() -5, "Galactic Center");
	labelsSup[1]->SetText(galMarksSup[1]->GetX(), galMarksSup[1]->GetY() -5, "TXS 0506+056");
	labelsSup[2]->SetText(galMarksSup[2]->GetX(), galMarksSup[2]->GetY() -5, "MRK 501");
	labelsSup[3]->SetText(galMarksSup[3]->GetX(), galMarksSup[3]->GetY() -5, "MRK 421");
	labelsSup[4]->SetText(galMarksSup[4]->GetX(), galMarksSup[4]->GetY() -5, "Virgo");
	labelsSup[5]->SetText(galMarksSup[5]->GetX(), galMarksSup[5]->GetY() -5, "Cygnus A");
	labelsSup[6]->SetText(galMarksSup[6]->GetX(), galMarksSup[6]->GetY() -5, "Cen A");
	labelsSup[7]->SetText(galMarksSup[7]->GetX(), galMarksSup[7]->GetY() -5, "Auger Dipole");
	labelsSup[8]->SetText(galMarksSup[8]->GetX(), galMarksSup[8]->GetY() -5, "Fornax");
	labelsSup[9]->SetText(galMarksSup[9]->GetX(), galMarksSup[9]->GetY() -5, "TA Hotspot");
	
	pad2->cd();
	
	for (int j=0;j<Nl;++j) latitudes[j]->Draw("l");
	for (int j=0;j<NL;++j) longitudes[j]->Draw("l");
	
	for(int i = 0; i < 10; i++) {
		galMarksSup[i]->Draw();
		labelsSup[i]->Draw();
	}
	
	//initializing and formatting graphical elements for the equatorial coordinate skymap
	TCanvas *skyProjCEq = new TCanvas("skyProjCEq","Skymap Projection (Equatorial Coordinates)",1500,750); //canvas for equatorial skymap projections
	
	skyProjCEq->cd(1);
	skyProjCEq->SetRightMargin(0.2);
	skymapProjEq->GetXaxis()->SetTitle("Right Ascension [deg]");
	skymapProjEq->GetYaxis()->SetTitle("Declination [deg]");
	skymapProjEq->GetZaxis()->SetTitle("Acceptance [cm^{2} s]");
	skymapProjEq->Draw("z aitoff");
	pad3->Draw();
	
	galMarksEq[0]->SetX(84.7649); //galactic center
	galMarksEq[0]->SetY(-32.1444);
	
	galMarksEq[1]->SetX(-77.0868); //TXS
	galMarksEq[1]->SetY(6.1484);
	
	galMarksEq[2]->SetX(86.8957); //MRK 501
	galMarksEq[2]->SetY(45.1052);
	
	galMarksEq[3]->SetX(-132.496); //MRK 421
	galMarksEq[3]->SetY(52.534);
	
	galMarksEq[4]->SetX(161.754); //Virgo
	galMarksEq[4]->SetY(-9.72109);
	
	galMarksEq[5]->SetX(49.3004); //cygnus A
	galMarksEq[5]->SetY(42.3715);
	
	galMarksEq[6]->SetX(119.23); // Cen A
	galMarksEq[6]->SetY(-56.6101);
	
	galMarksEq[7]->SetX(-92.1939); //Auger Dipole
	galMarksEq[7]->SetY(-29.1072);
	
	galMarksEq[8]->SetX(-44.1018); //Fornax
	galMarksEq[8]->SetY(-34.7217);
	
	galMarksEq[9]->SetX(-102.613); //TA Hotspot
	galMarksEq[9]->SetY(52.2853);
	
	labelsEq[0]->SetText(galMarksEq[0]->GetX(), galMarksEq[0]->GetY() -5, "Galactic Center");
	labelsEq[1]->SetText(galMarksEq[1]->GetX(), galMarksEq[1]->GetY() -5, "TXS 0506+056");
	labelsEq[2]->SetText(galMarksEq[2]->GetX(), galMarksEq[2]->GetY() -5, "MRK 501");
	labelsEq[3]->SetText(galMarksEq[3]->GetX(), galMarksEq[3]->GetY() +5, "MRK 421");
	labelsEq[4]->SetText(galMarksEq[4]->GetX(), galMarksEq[4]->GetY() -5, "Virgo");
	labelsEq[5]->SetText(galMarksEq[5]->GetX(), galMarksEq[5]->GetY() -5, "Cygnus A");
	labelsEq[6]->SetText(galMarksEq[6]->GetX(), galMarksEq[6]->GetY() -5, "Cen A");
	labelsEq[7]->SetText(galMarksEq[7]->GetX(), galMarksEq[7]->GetY() -5, "Auger Dipole");
	labelsEq[8]->SetText(galMarksEq[8]->GetX(), galMarksEq[8]->GetY() -5, "Fornax");
	labelsEq[9]->SetText(galMarksEq[9]->GetX(), galMarksEq[9]->GetY() -5, "TA Hotspot");
	
	pad3->cd();
	
	for (int j=0;j<Nl;++j) latitudes[j]->Draw("l");
	for (int j=0;j<NL;++j) longitudes[j]->Draw("l");
	
	for(int i = 0; i < 10; i++) {
		galMarksEq[i]->Draw();
		labelsEq[i]->Draw();
	}
	
	
	//Printing the specific acceptances and number of neutrino events of different galactic landmarks (commented out is for cross-checking acceptances over different coordinate systems to maintain consistancy)
	//~ cout<<"Equatorial:"<<endl;
	cout<<"Galacitc Center Acceptance: "<<skymapProjEq->GetBinContent((int)(93.5949 + 181), (int)(-28.9362 + 91))<<endl; 
	cout<<"TXS 0506+056 Acceptance: "<<skymapProjEq->GetBinContent((int)(-77.3581 + 181), (int)(5.69315 + 91))<<endl; 
	cout<<"MRK 501 Acceptance: "<<skymapProjEq->GetBinContent((int)(106.532 + 181), (int)(39.7602 + 91))<<endl; 
	cout<<"MRK 421 Acceptance: "<<skymapProjEq->GetBinContent((int)(-166.114 + 181), (int)(38.2089 + 91))<<endl; 
	cout<<"Virgo Acceptance: "<<skymapProjEq->GetBinContent((int)(162.797 + 181), (int)(-6.77748 + 91))<<endl; 
	cout<<"Cygnus A Acceptance: "<<skymapProjEq->GetBinContent((int)(60.1317 + 181), (int)(40.7339 + 91))<<endl; 
	cout<<"Cen A Acceptance: "<<skymapProjEq->GetBinContent((int)(158.635 + 181), (int)(-43.0192 + 91))<<endl; 
	cout<<"Auger Dipole Acceptance: "<<skymapProjEq->GetBinContent((int)(-99.7352 + 181), (int)(-25.7697 + 91))<<endl; 
	cout<<"Fornax Acceptance: "<<skymapProjEq->GetBinContent((int)(-50.1778 + 181), (int)(-33.73 + 91))<<endl; 
	cout<<"TA Hotspot Acceptance: "<<skymapProjEq->GetBinContent((int)(-133.503 + 181), (int)(43.1166 + 91))<<endl; 
	//~ cout<<"Supergalactic:"<<endl;
	//~ cout<<"Galacitc Center Acceptance: "<<skymapProjSuperGal->GetBinContent((int)(174.214 + 181), (int)(42.3103 + 91))<<endl; 
	//~ cout<<"TXS 0506+056 Acceptance: "<<skymapProjSuperGal->GetBinContent((int)(26.2644 + 181), (int)(-56.2196 + 91))<<endl; 
	//~ cout<<"MRK 501 Acceptance: "<<skymapProjSuperGal->GetBinContent((int)(-68.0962 + 181), (int)(54.3094 + 91))<<endl; 
	//~ cout<<"MRK 421 Acceptance: "<<skymapProjSuperGal->GetBinContent((int)(-71.5313 + 181), (int)(-10.5706 + 91))<<endl; 
	//~ cout<<"Virgo Acceptance: "<<skymapProjSuperGal->GetBinContent((int)(-123.91 + 181), (int)(1.46442 + 91))<<endl; 
	//~ cout<<"Cygnus A Acceptance: "<<skymapProjSuperGal->GetBinContent((int)(-0.443399 + 181), (int)(61.338 + 91))<<endl; 
	//~ cout<<"Cen A Acceptance: "<<skymapProjSuperGal->GetBinContent((int)(-159.753 + 181), (int)(-5.24987 + 91))<<endl; 
	//~ cout<<"Auger Dipole Acceptance: "<<skymapProjSuperGal->GetBinContent((int)(133.749 + 181), (int)(-79.2626 + 91))<<endl; 
	//~ cout<<"Fornax Acceptance: "<<skymapProjSuperGal->GetBinContent((int)(94.4711 + 181), (int)(-38.7337 + 91))<<endl; 
	//~ cout<<"TA Hotspot Acceptance: "<<skymapProjSuperGal->GetBinContent((int)(-50.0381 + 181), (int)(-25.1529 + 91))<<endl;
	//~ cout<<"Galactic:"<<endl;
	//~ cout<<"Galacitc Center Acceptance: "<<skymapFullProjection->GetBinContent((int)(-0 + 181), (int)(0 + 91))<<endl; 
	//~ cout<<"TXS 0506+056 Acceptance: "<<skymapFullProjection->GetBinContent((int)(164.595 + 181), (int)(-19.636 + 91))<<endl; 
	//~ cout<<"MRK 501 Acceptance: "<<skymapFullProjection->GetBinContent((int)(-63.6 + 181), (int)(38.8592 + 91))<<endl; 
	//~ cout<<"MRK 421 Acceptance: "<<skymapFullProjection->GetBinContent((int)(-179.832 + 181), (int)(65.0315 + 91))<<endl; 
	//~ cout<<"Virgo Acceptance: "<<skymapFullProjection->GetBinContent((int)(49.3718 + 181), (int)(55.8344 + 91))<<endl; 
	//~ cout<<"Cygnus A Acceptance: "<<skymapFullProjection->GetBinContent((int)(-76.1899 + 181), (int)(5.75539 + 91))<<endl; 
	//~ cout<<"Cen A Acceptance: "<<skymapFullProjection->GetBinContent((int)(50.4841 + 181), (int)(19.4173 + 91))<<endl; 
	//~ cout<<"Auger Dipole Acceptance: "<<skymapFullProjection->GetBinContent((int)(125 + 181), (int)(-14 + 91))<<endl; 
	//~ cout<<"Fornax Acceptance: "<<skymapFullProjection->GetBinContent((int)(126.161 + 181), (int)(-57.335 + 91))<<endl; 
	//~ cout<<"TA Hotspot Acceptance: "<<skymapFullProjection->GetBinContent((int)(-178 + 181), (int)(40 + 91))<<endl; 
	
	//printing the number of neutrino events in each source of interest
	cout<<"Galacitc Center Events: "<<nuevents->GetBinContent((int)(93.5949 + 181), (int)(-28.9362 + 91))<<endl; 
	cout<<"TXS 0506+056 Events: "<<nuevents->GetBinContent((int)(-77.3581 + 181), (int)(5.69315 + 91))<<endl; 
	cout<<"MRK 501 Events: "<<nuevents->GetBinContent((int)(106.532 + 181), (int)(39.7602 + 91))<<endl; 
	cout<<"MRK 421 Events: "<<nuevents->GetBinContent((int)(-166.114 + 181), (int)(38.2089 + 91))<<endl; 
	cout<<"Virgo Events: "<<nuevents->GetBinContent((int)(162.797 + 181), (int)(-6.77748 + 91))<<endl; 
	cout<<"Cygnus A Events: "<<nuevents->GetBinContent((int)(60.1317 + 181), (int)(40.7339 + 91))<<endl; 
	cout<<"Cen A Events: "<<nuevents->GetBinContent((int)(158.635 + 181), (int)(-43.0192 + 91))<<endl; 
	cout<<"Auger Dipole Events: "<<nuevents->GetBinContent((int)(-99.7352 + 181), (int)(-25.7697 + 91))<<endl; 
	cout<<"Fornax Events: "<<nuevents->GetBinContent((int)(-50.1778 + 181), (int)(-33.73 + 91))<<endl; 
	cout<<"TA Hotspot Events: "<<nuevents->GetBinContent((int)(-133.503 + 181), (int)(43.1166 + 91))<<endl;
	
	cout<<"Gamma ray burst acceptance (calculated per day): "<<gammarayAcc<<endl;
	cout<<"Gamma ray burst acceptance (calculated over 1 year): "<<totalAcc * (totalT / (360. * 365.)) / (4. * pi)<<endl;
}

Double_t GetEventSingleSrc(TH1D *hTau, Double_t minElog, Double_t maxElog)
{
	latitude = 38.52028; //lat of frisco peak, utah
	tStep = 2.5; //10 min step in degrees
	yMin = 20; //min distance from telescope where tau comes out of the ground in km
	yMax = 26;
	DeltaAngleAz = 0.1; //azimuth angle step
	DeltaAngle = 0.1; //elevation angle step
	MaxAzimuth = 180.; //max azi angle
	MaxElevation = 40; //max elv angle 
	bCombined = kTRUE; //both flor and cher events considered
	Double_t logEmin = minElog; //min energy log
    Double_t logEmax = maxElog; //max energy log
    Double_t LST = 0;
	Double_t degconv = pi/180.0;
	Double_t Enaught = 100000; //GeV from IceCube paper (100 TeV)
	Double_t Fnaught = 1.6e-18; //TeV^-1 cm^-2 s^-1 flux normalization at 100 TeV from IceCube paper over ~158 day period
	Double_t normInverse = (pow(pow(10, logEmin), (1 - nuIndex)) - pow(pow(10, logEmax), (1 - nuIndex))) / (nuIndex - 1); // integral of E^-nuIndex from Emin to Emax to correct for the normalization in the GetTauDistibution function
	//~ normInverse = 1.0;
	//~ multNorm = kFALSE;
	
	//values from differential sensitivity calculations
    yDelta = 5.0; //5
    iConfig = 2; //telescope altitude
    Double_t dFoV = 2;  //test 0, 1, 2, 10
    tanFoV = tan(dFoV/180.*pi);
    dFoVBelow =  3/180.*pi; 
    iMirrorSize = 2;
    dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 
    dMinLength = 0.3;
    
    TH2F *skymapSingleAngle = new TH2F("skymapSingleAngle111","Acceptance Skymap of Single Azimuth Angle [20 km to 150 km, 10^9 GeV]", 3601, -180.05, 180.05, 1801, -90.05, 90.05); //histo for single angle acceptance plot
	TH2F *skymapFull360Sweep = new TH2F("skymapFull360Sweep111","Acceptance Skymap of 360 Degree Airshower Azimuth Sweep [20 km to 150 km, 10^9 GeV]", 3601, -180.05, 180.05, 1801, -90.05, 90.05);
	TH2F *skymapProjEq = new TH2F("skymapProjEq111","360 FoV Projection In Equatorial Coordinates Over MJD 56937.81 - 57096.21 [10^9 GeV]", 361, -180.05, 180.05, 181, -90.05, 90.05); //equatorial
	TH2F *nuevents = (TH2F*)skymapProjEq->Clone("nuevents111");
	
	skymapSingleAngle->Reset("ICESM");
	skymapFull360Sweep->Reset("ICESM");
	skymapProjEq->Reset("ICESM");
	nuevents->Reset("ICESM");
	
	GetAcceptanceSingleAngle(logEmin, logEmax, hTau, skymapSingleAngle);
	
	for(int yBins = 1; yBins <= skymapSingleAngle->GetNbinsY(); yBins++)
    {
		Double_t comboBin = 0;
		for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
			comboBin += skymapSingleAngle->GetBinContent(xBins, yBins);
		for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
			skymapFull360Sweep->SetBinContent(xBins, yBins, comboBin);
	}
	
	ifstream in;
	in.open("txsflare.txt"); //open ephem file
	
	Double_t totalT = 0, totalAcc = 0;
	//calculations for the time evolution of the horizontal skymaps over various coordinate systems
	if (in.is_open())
	{
		Double_t setTimeSun, riseTimeSun, riseTimeMoon, setTimeMoon, phaseMoon, deltaT = 0, tStepAdjusted; 
		//~ Double_t origLST;
		string sTimeS, rTimeS, rTimeM, sTimeM, phM;
		bool nestNone = false, nestSun = false, nestBoth = false;
		int nSteps;
		
		while(in.good()) 
		{
			in >> sTimeS >> rTimeS >> rTimeM >> sTimeM >> phM; //read from the file that contains rise and set times of moon and sun in LST as well as phase of moon
			
			setTimeSun = stod(sTimeS);
			riseTimeSun = stod(rTimeS);
			riseTimeMoon = stod(rTimeM);
			setTimeMoon = stod(sTimeM);
			phaseMoon = stod(phM);
			
			nestNone = false;
			nestSun = false;
			nestBoth = false;
			
			//moon phase calculations
			if(phaseMoon < 0.3) {
				if(setTimeSun > riseTimeSun)
					deltaT = (360.0 - setTimeSun) + riseTimeSun;
				else
					deltaT = riseTimeSun - setTimeSun;
				LST = setTimeSun;
			} else {
				if( (riseTimeSun > setTimeSun) && (setTimeMoon > riseTimeMoon) ) { //both don't cross 0hr
					if( (riseTimeMoon < setTimeSun) && (setTimeMoon < setTimeSun) && (setTimeMoon < riseTimeSun) ){
						LST = setTimeMoon;
						deltaT = riseTimeSun - setTimeMoon;
					} else if( (riseTimeMoon > setTimeSun) && (setTimeMoon > riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeMoon - setTimeSun;
					} else if( (riseTimeMoon > setTimeSun) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (riseTimeMoon - setTimeSun) + (riseTimeSun - setTimeMoon);
						nestNone = true;
					} else if( (riseTimeMoon < setTimeSun && setTimeMoon < setTimeSun) || (riseTimeMoon > riseTimeSun && setTimeMoon > riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeSun - setTimeSun;
					} else { deltaT = 0; }
				} else if( (riseTimeSun < setTimeSun) && (setTimeMoon > riseTimeMoon) ) { //only sun crosses 0hr (impossible for moon to cover full night)
					if( (riseTimeMoon < setTimeSun) && (setTimeMoon > setTimeSun) ) {
						LST = setTimeMoon;
						deltaT = (360. + riseTimeSun) - setTimeMoon;
					} else if( (riseTimeSun > riseTimeMoon) && (riseTimeSun < setTimeMoon) ) {
						LST = setTimeSun;
						deltaT = (360. + riseTimeMoon) - setTimeSun;
					} else if( (riseTimeMoon > setTimeSun && setTimeMoon < 360.) || (riseTimeMoon < riseTimeSun && setTimeMoon < riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (riseTimeSun + 360. - setTimeSun) - (setTimeMoon - riseTimeMoon);
						nestSun = true;
					} else if( (riseTimeMoon < setTimeSun && setTimeMoon < setTimeSun) || (riseTimeMoon > riseTimeSun && setTimeMoon > riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (360. + riseTimeSun) - setTimeSun;
					}
				} else if( (riseTimeSun > setTimeSun) && (setTimeMoon < riseTimeMoon) ) { //only moon crosses 0hr
					if( ((setTimeSun + 360.) > riseTimeMoon) && (setTimeMoon > setTimeSun) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeMoon;
						deltaT = riseTimeSun - setTimeMoon;
					} else if( (riseTimeMoon < riseTimeSun) && (riseTimeSun < (setTimeMoon + 360.)) && (riseTimeMoon > setTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeMoon - setTimeSun;
					} else if( (riseTimeMoon > riseTimeSun) && (setTimeMoon < setTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeSun - setTimeSun;
					} else { deltaT = 0; }
				} else if ( (riseTimeSun < setTimeSun)  && (setTimeMoon < riseTimeMoon) ) { //both cross 0 hr
					if( (riseTimeMoon > setTimeSun) && (riseTimeSun < setTimeMoon) ) {
						LST = setTimeSun;
						deltaT = riseTimeMoon - setTimeSun;
					} else if( (riseTimeMoon < setTimeSun) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeMoon;
						deltaT = riseTimeSun - setTimeMoon;
					} else if( (setTimeSun < riseTimeMoon) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (riseTimeSun + 360. - setTimeSun) - (setTimeMoon + 360. - riseTimeMoon);
						nestBoth = true;
					} else { deltaT = 0; }
				}
			}
			
			totalT += deltaT;
			nSteps = (int)(deltaT / tStep);
			tStepAdjusted = deltaT / nSteps;

			//evolving the horizotal skymap over time and projecting onto equatorial coordinates
			for(int i = 0; i < nSteps; i++) { //eqatorial
				for(int r = -180; r <= 180; r++) {
					for(int d = -90; d <= 90; d++) {
						Double_t az = (atan2(sin((LST - r) * degconv), cos((LST - r) * degconv) * sin(latitude * degconv) - tan(d * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
						Double_t alt = asin(sin(latitude * degconv) * sin(d * degconv) + cos(latitude * degconv) * cos(d * degconv) * cos((LST - r) * degconv)) * 180 / pi;
						if(az > 180.0)
							az = az - 360.0;
						else if(az < -180.0)
							az = az + 360.0;
						int xBin = (int)((az + 180.1) * 10);
						int yBin = (int)((alt + 90.1) * 10);
						if( !(nestNone && LST > riseTimeMoon && LST < setTimeMoon) && 
							!(nestSun && LST > riseTimeMoon && LST < setTimeMoon) && 
							!(nestBoth && ( (LST > riseTimeMoon && LST < 360.) || (LST > 0 && LST < setTimeMoon) ) ) ) 
							{ 
								skymapProjEq->Fill((-1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin) * tStepAdjusted * 240); //240 sec = 1 degree of RA
								//~ nuevents->Fill((-1 * r), d, skymapFull360Sweep->GetBinContent(xBin, yBin) * tStepAdjusted * 240 * normInverse * Fnaught / pow(Enaught, -nuIndex));
							} 
					}
				}
				LST += tStepAdjusted;
				if(LST > 360.0)
					LST -= 360.0;
			}
		}
		//applying the IceCube parameters to the time-evolved acceptance to get the number of neutrino events in each bin
		for(int i = 1; i <= skymapProjEq->GetNbinsX(); i++)
		{
			for(int j = 1; j <= skymapProjEq->GetNbinsY(); j++)
				nuevents->SetBinContent(i, j, skymapProjEq->GetBinContent(i, j) * normInverse * Fnaught / pow(Enaught, -nuIndex));
		}
		
		in.close();
	} else {cout << "Unable to open file" << endl; return -1.0; }
	
	for(int i = 1; i <= skymapProjEq->GetNbinsX(); i++) {
		for(int j = 1; j <= skymapProjEq->GetNbinsY(); j++)
			totalAcc += skymapProjEq->GetBinContent(i, j);
	}
	
	//printing observation time, acceptance, and duty cycle
	//~ cout<<"Total observation time: "<<totalT * (24.0 / 360.0)<<" hours."<<endl;
	//~ cout<<"Total Acceptance over "<<totalT * (24.0 / 360.0)<<" hours: "<<totalAcc<<endl;
	
	Double_t out = nuevents->GetBinContent((int)(-77.3581 + 181), (int)(5.69315 + 91));
	
	delete(skymapSingleAngle);
	delete(skymapFull360Sweep);
	delete(skymapProjEq);
	delete(nuevents);
	
	return out;
}

Double_t PrintAccSrc(TH1D *hTau, Double_t minElog, Double_t maxElog)
{
	latitude = 38.52028; //lat of frisco peak, utah
	tStep = 2.5; //10 min step in degrees
	yMin = 20; //min distance from telescope where tau comes out of the ground in km
	yMax = 26;
	DeltaAngleAz = 0.1; //azimuth angle step
	DeltaAngle = 0.1; //elevation angle step
	MaxAzimuth = 180.; //max azi angle
	MaxElevation = 40; //max elv angle 
	bCombined = kTRUE; //both flor and cher events considered
	Double_t logEmin = minElog; //min energy log
    Double_t logEmax = maxElog; //max energy log
    Double_t LST = 0;
	Double_t degconv = pi/180.0;
	Double_t srcRA = 77.35811176;
	Double_t srcDec = 5.69314237;
	
	//values from differential sensitivity calculations
    yDelta = 5.0; //5
    iConfig = 2; //telescope altitude
    Double_t dFoV = 2;  //test 0, 1, 2, 10
    tanFoV = tan(dFoV/180.*pi);
    dFoVBelow =  3/180.*pi; 
    iMirrorSize = 2;
    dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 
    dMinLength = 0.3;
    //~ multNorm = kFALSE;
    
    TH2F *skymapSingleAngle = new TH2F("skymapSingleAngle111","Acceptance Skymap of Single Azimuth Angle [20 km to 150 km, 10^9 GeV]", 3601, -180.05, 180.05, 1801, -90.05, 90.05); //histo for single angle acceptance plot
	TH2F *skymapFull360Sweep = new TH2F("skymapFull360Sweep111","Acceptance Skymap of 360 Degree Airshower Azimuth Sweep [20 km to 150 km, 10^9 GeV]", 3601, -180.05, 180.05, 1801, -90.05, 90.05);
	
	GetAcceptanceSingleAngle(logEmin, logEmax, hTau, skymapSingleAngle);
	
	for(int yBins = 1; yBins <= skymapSingleAngle->GetNbinsY(); yBins++)
    {
		Double_t comboBin = 0;
		for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
			comboBin += skymapSingleAngle->GetBinContent(xBins, yBins);
		for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
			skymapFull360Sweep->SetBinContent(xBins, yBins, comboBin);
	}
	
	ifstream in;
	in.open("txsflare.txt"); //open ephem file
	
	Double_t totalAcc = 0, totalT = 0;
	int nCrosses = 0;
	if (in.is_open())
	{
		Double_t setTimeSun, riseTimeSun, riseTimeMoon, setTimeMoon, phaseMoon, deltaT = 0, tStepAdjusted; 
		string sTimeS, rTimeS, rTimeM, sTimeM, phM;
		bool nestNone = false, nestSun = false, nestBoth = false;
		int nSteps;
		
		while(in.good()) 
		{
			in >> sTimeS >> rTimeS >> rTimeM >> sTimeM >> phM; //read from the file that contains rise and set times of moon and sun in LST as well as phase of moon
			
			setTimeSun = stod(sTimeS);
			riseTimeSun = stod(rTimeS);
			riseTimeMoon = stod(rTimeM);
			setTimeMoon = stod(sTimeM);
			phaseMoon = stod(phM);
			
			nestNone = false;
			nestSun = false;
			nestBoth = false;
			
			//moon phase calculations
			if(phaseMoon < 0.3) {
				if(setTimeSun > riseTimeSun)
					deltaT = (360.0 - setTimeSun) + riseTimeSun;
				else
					deltaT = riseTimeSun - setTimeSun;
				LST = setTimeSun;
			} else {
				if( (riseTimeSun > setTimeSun) && (setTimeMoon > riseTimeMoon) ) { //both don't cross 0hr
					if( (riseTimeMoon < setTimeSun) && (setTimeMoon < setTimeSun) && (setTimeMoon < riseTimeSun) ){
						LST = setTimeMoon;
						deltaT = riseTimeSun - setTimeMoon;
					} else if( (riseTimeMoon > setTimeSun) && (setTimeMoon > riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeMoon - setTimeSun;
					} else if( (riseTimeMoon > setTimeSun) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (riseTimeMoon - setTimeSun) + (riseTimeSun - setTimeMoon);
						nestNone = true;
					} else if( (riseTimeMoon < setTimeSun && setTimeMoon < setTimeSun) || (riseTimeMoon > riseTimeSun && setTimeMoon > riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeSun - setTimeSun;
					} else { deltaT = 0; }
				} else if( (riseTimeSun < setTimeSun) && (setTimeMoon > riseTimeMoon) ) { //only sun crosses 0hr (impossible for moon to cover full night)
					if( (riseTimeMoon < setTimeSun) && (setTimeMoon > setTimeSun) ) {
						LST = setTimeMoon;
						deltaT = (360. + riseTimeSun) - setTimeMoon;
					} else if( (riseTimeSun > riseTimeMoon) && (riseTimeSun < setTimeMoon) ) {
						LST = setTimeSun;
						deltaT = (360. + riseTimeMoon) - setTimeSun;
					} else if( (riseTimeMoon > setTimeSun && setTimeMoon < 360.) || (riseTimeMoon < riseTimeSun && setTimeMoon < riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (riseTimeSun + 360. - setTimeSun) - (setTimeMoon - riseTimeMoon);
						nestSun = true;
					} else if( (riseTimeMoon < setTimeSun && setTimeMoon < setTimeSun) || (riseTimeMoon > riseTimeSun && setTimeMoon > riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (360. + riseTimeSun) - setTimeSun;
					}
				} else if( (riseTimeSun > setTimeSun) && (setTimeMoon < riseTimeMoon) ) { //only moon crosses 0hr
					if( ((setTimeSun + 360.) > riseTimeMoon) && (setTimeMoon > setTimeSun) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeMoon;
						deltaT = riseTimeSun - setTimeMoon;
					} else if( (riseTimeMoon < riseTimeSun) && (riseTimeSun < (setTimeMoon + 360.)) && (riseTimeMoon > setTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeMoon - setTimeSun;
					} else if( (riseTimeMoon > riseTimeSun) && (setTimeMoon < setTimeSun) ) {
						LST = setTimeSun;
						deltaT = riseTimeSun - setTimeSun;
					} else { deltaT = 0; }
				} else if ( (riseTimeSun < setTimeSun)  && (setTimeMoon < riseTimeMoon) ) { //both cross 0 hr
					if( (riseTimeMoon > setTimeSun) && (riseTimeSun < setTimeMoon) ) {
						LST = setTimeSun;
						deltaT = riseTimeMoon - setTimeSun;
					} else if( (riseTimeMoon < setTimeSun) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeMoon;
						deltaT = riseTimeSun - setTimeMoon;
					} else if( (setTimeSun < riseTimeMoon) && (setTimeMoon < riseTimeSun) ) {
						LST = setTimeSun;
						deltaT = (riseTimeSun + 360. - setTimeSun) - (setTimeMoon + 360. - riseTimeMoon);
						nestBoth = true;
					} else { deltaT = 0; }
				}
			}
			
			totalT += deltaT;
			nSteps = (int)(deltaT / tStep);
			tStepAdjusted = deltaT / nSteps;

			//evolving the horizotal skymap over time and projecting onto equatorial coordinates
			for(int i = 0; i < nSteps; i++) { //eqatorial
				Double_t az = (atan2(sin((LST - srcRA) * degconv), cos((LST - srcRA) * degconv) * sin(latitude * degconv) - tan(srcDec * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
				Double_t alt = asin(sin(latitude * degconv) * sin(srcDec * degconv) + cos(latitude * degconv) * cos(srcDec * degconv) * cos((LST - srcRA) * degconv)) * 180 / pi;
				if(az > 180.0)
					az = az - 360.0;
				else if(az < -180.0)
					az = az + 360.0;
				int xBin = (int)((az + 180.1) * 10);
				int yBin = (int)((alt + 90.1) * 10);
				if( !(nestNone && LST > riseTimeMoon && LST < setTimeMoon) && 
					!(nestSun && LST > riseTimeMoon && LST < setTimeMoon) && 
					!(nestBoth && ( (LST > riseTimeMoon && LST < 360.) || (LST > 0 && LST < setTimeMoon) ) ) ) 
					{ 
						if(skymapFull360Sweep->GetBinContent(xBin, yBin) > 0) {
							cout<<"Instantaneous acceptance at source: "<<skymapFull360Sweep->GetBinContent(xBin, yBin)<<" cm^2 for energies 10^"<<logEmin<<" GeV to 10^"<<logEmax<<" GeV."<<endl;
							totalAcc += skymapFull360Sweep->GetBinContent(xBin, yBin);
							nCrosses++;
						}
					} 
				LST += tStepAdjusted;
				if(LST > 360.0)
					LST -= 360.0;
			}
		}
		cout<<"Total acceptance at source: "<<totalAcc<<" cm^2 for energies 10^"<<logEmin<<" GeV to 10^"<<logEmax<<" GeV."<<endl;
		cout<<"Total times source was observed in the FoV over "<<totalT * (24.0 / 360.0)<<" hour observational period: "<<nCrosses<<endl; 
		in.close();
		
		delete(skymapSingleAngle);
		delete(skymapFull360Sweep);
		
		return totalAcc;
	} else {cout << "Unable to open file" << endl; return -1.0; }
}

void GetEventVEnergy(TH1D *hTau)
{
	Double_t min = 6.0;
	Double_t max = 10.0;
	Double_t step = 0.1;
	Double_t eng = min;
	int nSteps = (int)((max - min) / step);
	Double_t totalEvents = 0;
	
	//~ multNorm = kFALSE;
	//~ cout<<"Without normalization constant: "<<endl;
	//~ for(int i = 0; i < nSteps; i++)
	//~ {
		//~ Double_t nev = GetEventSingleSrc(hTau, eng, eng + step);
		//~ cout<<"Number of events between 10^"<<eng<<" GeV and 10^"<<eng + step<<" Gev: "<<nev<<endl;
		//~ eng += step;
		//~ totalEvents += nev;
	//~ }
	//~ cout<<"Total events between 10^"<<min<<" GeV and 10^"<<max<<" Gev: "<<totalEvents<<endl<<endl;
	
	multNorm = kTRUE;
	cout<<"With normalization constant: "<<endl;
	totalEvents = 0;
	eng = min;
	for(int i = 0; i < nSteps; i++)
	{
		Double_t nev = GetEventSingleSrc(hTau, eng, eng + step);
		cout<<"Number of events between 10^"<<eng<<" GeV and 10^"<<eng + step<<" Gev: "<<nev<<endl;
		eng += step;
		totalEvents += nev;
	}
	cout<<"Total events between 10^"<<min<<" GeV and 10^"<<max<<" Gev: "<<totalEvents<<endl<<endl;
}

void PlotSrcInstantAccVsE(TH1D *hTau)
{
	Double_t min = 6.0;
	Double_t max = 10.0;
	Double_t step = 0.1;
	Double_t En = min;
	int nSteps = (int)((max - min) / step);
	Double_t eng[nSteps], acc[nSteps];
	Double_t totalA = 0;
	
	for(int i = 0; i < nSteps; i++) {
		eng[i] = pow(10.0, En);
		acc[i] = PrintAccSrc(hTau, En, (En + step));
		
		totalA += acc[i];
		En += step;
	}
	
	TCanvas *accVE = new TCanvas("accVE", "Acceptance v. Neutrino Energy", 1300, 900);
    TGraph *accVSEplot = new TGraph(nSteps, eng, acc);
    accVSEplot->SetLineColor(2);
	accVSEplot->SetLineWidth(4);
	accVSEplot->SetTitle("Plot of Acceptance Vs. Neutino Energy");
	accVSEplot->GetYaxis()->SetTitle("Acceptance [cm^{2}]");
	accVSEplot->GetXaxis()->SetTitle("Neutrino Energy [GeV]");
	accVE->SetLogy(1);
	accVE->SetLogx(1);
	accVE->SetGridx(1);
	accVE->SetGridy(1);
	accVSEplot->Draw("AC");
	
	cout<<"Total instant acceptance of source: "<<totalA<<" cm^2 over evergy range 10^"<<min<<" GeV to 10^"<<max<<" GeV."<<endl;
}

void SrcEventTest(TH1D *hTau)
{
	latitude = 38.52028; //lat of frisco peak, utah
	tStep = 2.5; //10 min step in degrees
	yMin = 20; //min distance from telescope where tau comes out of the ground in km
	yMax = 26;
	DeltaAngleAz = 0.1; //azimuth angle step
	DeltaAngle = 0.1; //elevation angle step
	MaxAzimuth = 180.; //max azi angle
	MaxElevation = 40; //max elv angle 
	bCombined = kTRUE; //both flor and cher events considered
	Double_t logEmin = 6.0; //min energy log
    Double_t logEmax = 10.0; //max energy log
    Double_t LST = 82;
	Double_t degconv = pi/180.0;
	Double_t srcRA = 77.35811176;
	Double_t srcDec = 5.69314237;
	Double_t Enaught = 100000; //GeV from IceCube paper (100 TeV)
	Double_t Fnaught = 1.6e-18; //TeV^-1 cm^-2 s^-1 flux normalization at 100 TeV from IceCube paper over ~158 day period
	Double_t normInverse = (pow(pow(10, logEmin), (1 - nuIndex)) - pow(pow(10, logEmax), (1 - nuIndex))) / (nuIndex - 1); // integral of E^-nuIndex from Emin to Emax to correct for the normalization in the GetTauDistibution function
	
	//values from differential sensitivity calculations
    yDelta = 5.0; //5
    iConfig = 2; //telescope altitude
    Double_t dFoV = 2;  //test 0, 1, 2, 10
    tanFoV = tan(dFoV/180.*pi);
    dFoVBelow =  3/180.*pi; 
    iMirrorSize = 2;
    dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 
    dMinLength = 0.3;
    //~ multNorm = kFALSE;
    
    TH2F *skymapSingleAngle = new TH2F("skymapSingleAngle111","Acceptance Skymap of Single Azimuth Angle [20 km to 150 km, 10^9 GeV]", 3601, -180.05, 180.05, 1801, -90.05, 90.05); //histo for single angle acceptance plot
	TH2F *skymapFull360Sweep = new TH2F("skymapFull360Sweep111","Acceptance Skymap of 360 Degree Airshower Azimuth Sweep [20 km to 150 km, 10^9 GeV]", 3601, -180.05, 180.05, 1801, -90.05, 90.05);
	
	GetAcceptanceSingleAngle(logEmin, logEmax, hTau, skymapSingleAngle);
	
	for(int yBins = 1; yBins <= skymapSingleAngle->GetNbinsY(); yBins++)
    {
		Double_t comboBin = 0;
		for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
			comboBin += skymapSingleAngle->GetBinContent(xBins, yBins);
		for(int xBins = 1; xBins <= skymapSingleAngle->GetNbinsX(); xBins++)
			skymapFull360Sweep->SetBinContent(xBins, yBins, comboBin);
	}
	
	Double_t acc = 0, totalacc = 0, tcross = 0;
	
	for(int i = 0; i < (int)((254 - 82) / tStep); i++)
	{
		Double_t az = (atan2(sin((LST - srcRA) * degconv), cos((LST - srcRA) * degconv) * sin(latitude * degconv) - tan(srcDec * degconv) * cos(latitude * degconv)) * 180 / pi) - 180;
		Double_t alt = asin(sin(latitude * degconv) * sin(srcDec * degconv) + cos(latitude * degconv) * cos(srcDec * degconv) * cos((LST - srcRA) * degconv)) * 180 / pi;
		if(az > 180.0)
			az = az - 360.0;
		else if(az < -180.0)
			az = az + 360.0;
		int xBin = (int)((az + 180.1) * 10);
		int yBin = (int)((alt + 90.1) * 10);
		
		if(skymapFull360Sweep->GetBinContent(xBin, yBin) > 0)
		{
			acc += skymapFull360Sweep->GetBinContent(xBin, yBin) * tStep * 240.0;
			tcross += tStep;
		}
		
		LST += tStep;
	}
	
	totalacc = acc * 158.0;
	
	cout<<"Acceptance from source from single night observation: "<<acc<<endl;
	cout<<"Extrapolated acceptance from 158 days of observation: "<<totalacc<<endl;
	cout<<"Amount of time the souce was in view for single observation: "<<tcross * (24.0 / 360.0)<< "hours"<<endl;
	cout<<"Amount of time the souce was in view for 158 days of observation: "<<tcross * (24.0 / 360.0) * 158.0<<" hours"<<endl;
	cout<<"Total number of events from source over 158 days of observation: "<<totalacc * normInverse * Fnaught / pow(Enaught, -nuIndex)<<endl;
	
}

////////////////////////////////////////////////////////////////////
// Main Program
//
//////////////////////////////////////////////////////////////////
int main (int argc, char **argv) {


  //initiate root
  TROOT root("DisplayEvts","Display Results");
  TApplication *theApp = new TApplication("App",&argc,argv);
  gROOT->ProcessLine("#include <vector>"); //need this otherwise we cannot save vectors in the root file

  //read tables to use NuTauSimResults	
  readFromTable() ;
  findAngleNumber() ;
                          
//cout<<PDecayFluorescence(1e9,10,3,170)<<endl;;

fPE = new TF1("fPE",myPEfunction,0,40,2);

//TLegend *leg = new TLegend(0.14,0.6,0.31,0.87);
grsCC = new TGraph(19, Esig, sigma);
grsNC = new TGraph(19, Esig, sigmaNC);

hTriggeredAzimuthAngles = new TH1D("hTriggeredAzimuthAngles","",int(180/DeltaAngleAz),0,180);
hTriggeredAzimuthAngles->GetXaxis()->SetTitle("angle #alpha [degrees]");
hTriggeredAzimuthAngles->GetYaxis()->SetTitle("probability");
hTriggeredAzimuthAngles->SetLineWidth(2);
hTriggeredAzimuthAngles->SetLineColor(kBlue+3);

//plot the probability that a tau emerges when a neutrino hits the Earth
PlotEmergenceProbability();
cout<<"Done with Emergence Probability Plot"<<endl;
//Double_t dmin = 1;
//Double_t dmax = 6001.0; //thickness in km
//Double_t xStepCoarse = 0.1;

gStyle->SetOptStat(0);

TH1D *hTau = new TH1D("hTau","",70,4,11);
hTau->GetXaxis()->SetTitle("energy [GeV]");
hTau->GetYaxis()->SetTitle("F_tau/F_nu");
hTau->SetLineWidth(3);
hTau->GetXaxis()->SetLabelSize(0.045);
hTau->GetXaxis()->SetTitleOffset(1.5);
hTau->GetYaxis()->SetTitleOffset(1.3);
hTau->GetYaxis()->SetTitleSize(0.04);
hTau->GetXaxis()->SetTitleSize(0.04);
hTau->GetYaxis()->SetLabelSize(0.045);

TAxis *axis = hTau->GetXaxis();
int bins = axis->GetNbins();
Axis_t from = axis->GetXmin();
Axis_t to = axis->GetXmax();
Axis_t width = (to - from) / bins;
Axis_t *new_bins = new Axis_t[bins + 1];
for (int i = 0; i <= bins; i++) {
     new_bins[i] = TMath::Power(10, from + i * width);
}
axis->Set(bins, new_bins);

//making Fta/Fnu plot from dutta paper
for(int i=0;i<hTau->GetNbinsX();i++)
{
  Double_t El = hTau->GetBinLowEdge(i+1);
  Double_t Eh = hTau->GetBinLowEdge(i+2);
  Double_t Enu=hTau->GetBinCenter(i+1);

  Double_t weight = log(Eh)-log(El) ;

  Double_t Etau = hTau->GetBinCenter(1); 
  int n = 0;
  while(Etau<=Eh)
  {
   //  cout<<"Energy "<<E<<endl;
   Double_t P = PEtau(100,Etau,Enu,hTau) ;
   hTau->Fill(Etau,weight*P);
   n++;
   Etau=  hTau->GetBinCenter(n+1);
  }
}

//convert to different units
for(int i=0;i<hTau->GetNbinsX();i++)
{
   //Double_t Enu = hTau->GetBinCenter(i+1);
   Double_t El = hTau->GetBinLowEdge(i+1);
   Double_t Eh = hTau->GetBinLowEdge(i+2);
   hTau->SetBinContent(i+1,hTau->GetBinContent(i+1)/(log(Eh)-log(El)));
   hTau->SetBinError(i+1,0);
}

//plot clone of hTau


//produce plot with tau energy distribution for four different target
//thicknesses

TCanvas *cTauSpeMonoEn = new TCanvas("cTauSpeMonoEn","Energy Distribution of Taus produced by 10^9 GeV Nus",750,500);
cTauSpeMonoEn->Draw();
cTauSpeMonoEn->SetLogx();
cTauSpeMonoEn->SetLogy();

hTau->GetYaxis()->SetTitle("probability of #tau emergence");

TLegend *legend = new TLegend(0.75,0.6,0.97,0.95);
TString legstr;
for(float i=0;i<4;i++)
{
GetTauDistribution(hTau,pow(10,i),pow(10,9.0),pow(10,9.1001));//9.0...9.1
TH1D *h = (TH1D*)hTau->Clone("h");
h->SetLineStyle(i+1);
h->Draw("same");
legstr.Form("%0.0f km",pow(10,i));
legend->AddEntry(h,legstr.Data(),"l");
}
legend->Draw();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Sensitivity calculation starts here
bFluorescence = kFALSE;

//CalculateAcceptanceVsImageLength(hTau);

//CalculateAcceptanceVsUpperFoV(hTau);
//
//CalculateAcceptanceVsLowerFoV(hTau);

//CalculateAcceptanceVsTelescopeHeight(hTau);

//CalculateAcceptanceVsThreshold(hTau);
//
//CalculateAcceptanceVsEnergy(hTau);
//
//CalculateIntegralSensitivity(hTau);
//~ CalculateDifferentialSensitivity(hTau);
//~ CalculateSkyExposure(hTau);
//

PlotAcceptanceSkymaps(hTau);
//~ PlotAcceptanceVsEnergy(hTau);
//~ GetEventVEnergy(hTau);
//~ PrintAccSrc(hTau, 6.0, 6.5);
//~ PlotSrcInstantAccVsE(hTau);
//~ SrcEventTest(hTau);

/*
cout<<"DEBUGGING"<<endl;
               Double_t dEarth = DistanceThroughEarth(5,0.75);
               //GetTauDistribution(hTau,dEarth,9.3,10.3);                
               GetTauDistribution(hTau,dEarth,9.5,10.5);                
               for(int i=0;i<hTau->GetNbinsX();i++)
                  {
                     cout<<hTau->GetBinCenter(i+1)  <<" taus cont: "<<hTau->GetBinContent(i+1)<<endl;
                  }

*/
/*
yMin = 5;
yMax = 5.1;
yDelta = 5;
MaxElevation = 30;
DeltaAngle = 0.05;
dMaxCherenkovAzimuthAngle = 30;
dMinEnu = 5.5;
dMaxEnu = 6.5;

    Double_t dFoV = 3;  //test 0, 1, 2, 10
    tanFoV = tan(dFoV/180.*pi);
    //dFoVBelow = asin(REarth/(REarth+DetectorAltitude[iConfig]));
    dFoVBelow =  2/180.*pi; 
    iMirrorSize = 2;
    dMinimumNumberPhotoelectrons = dThreshold[iMirrorSize]/dMirrorA[iMirrorSize]; 

    dMinLength = 0.3; //mimnimum length a shower has to have in the camera, in degrees. This is a conservative estimate because it assumes that the shower starts at a distance l from the detector, which is not necessarily tru for showers with shallow elevation angles.
TGraph *grTMP = new TGraph();
CalculateAcceptance(dMinEnu,dMaxEnu,grTMP,hTau);
*/

//loop over threshold energy//
//

//loop over nu energy bins weighting with power law with index g and fill spectrum with tau energies
//Generate Tau spectrum for neutrinos in this bin emerging under a given angle
//
/*
TCanvas *cSens = new TCanvas("cSens","Sensitivity",750,500);
cSens->Draw();
TH1D *hSens = new TH1D("hSens","",1000,0.9*dmin,dmax*1.1);
hSens->Draw();
hSens->SetMaximum(1e-3);
hSens->SetMinimum(1e-10);
hSens->GetXaxis()->SetTitle("Target thickness [km]");
hSens->GetYaxis()->SetTitle("Sensitivity [GeV cm^{-2} s^{-1} sr^{-1}]");
cSens->SetLogx();
cSens->SetLogy();
TMultiGraph *mGrSens = new TMultiGraph("mGrSens","MGrSens");


cSens->cd();
mGrSens->Draw("PL");
leg->Draw();
leg->Draw();
*/


/*
 TTimer timer("gSystem->ProcessEvents();", 50, kFALSE);
   timer.TurnOn();
   Getline("Type <return> to go on: ");
   timer.TurnOff();
*/
  cout<<"done"<<endl;
  theApp->Run();

return 0;
}
