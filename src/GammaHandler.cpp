#include "GammaHandler.hh"


using namespace std;

double yMax = 1.0; //arbitrary y max, might need to change
double brThresh = 0.1;


vector<vector<double>> GammaHandler::sourceLookupTable(string source) {
	//energy container vector orginized as {energy, branch ratio, PE mass attenuation coef, Compton coef, PP coef}
	vector<vector<double>> returnInfo;
	if(source == "Co57") {
		vector<double> e1 = {122.0, 0.856, 1.793, 0.1081, 0.00};
		vector<double> e2 = {136.0, 0.1068, 0.5651, 0.1019, 0.00};
		vector<double> e3 = {14.0, 0.0916, 55.5, 0.0744, 0.00};
		returnInfo.push_back(e1);
		returnInfo.push_back(e2);
		returnInfo.push_back(e3);
		return returnInfo;
	}else if(source == "Co60") {
		vector<double> e1 = {1332.0, 0.9998, 0.001991, 0.04244, 0.0008853};
		vector<double> e2 = {1173.0, 0.9985, 0.004126, 0.05156, 0.00};
		returnInfo.push_back(e1);
		returnInfo.push_back(e2);
		return returnInfo;
	 }else if(source == "Cs137") {
		vector<double> e1 = {662.0, 0.851, 0.01338, 0.06559, 0.00};
		vector<double> e2 = {284.0, 0.0006, 0.08009, 0.08495, 0.00};
		returnInfo.push_back(e1);
		returnInfo.push_back(e2);
		return returnInfo;

	}
	cerr << source << " Is not a valid option!" << endl;
	exit(1);
	return returnInfo;
}


double GammaHandler::combineSpectra(double emin, double emax, string source) {
	double brSum = 0.0;
	double fValue = 0.0;

	vector<vector<double>> sourceInfo = GammaHandler::sourceLookupTable(source);

	vector<double> xyTry = {
      emin + (emax - emin) * RandomGen::rndm()->rand_uniform(),
      yMax * RandomGen::rndm()->rand_uniform(), 1.};

     while(xyTry[2] > 0.) {
     	
     		fValue = GammaHandler::photoIonization(sourceInfo, xyTry) + 
     		GammaHandler::compton(sourceInfo, xyTry) + 
     		GammaHandler::pairProduction(sourceInfo, xyTry);
     	

     	xyTry = RandomGen::rndm()->VonNeumann(emin, emax, 0., yMax, xyTry[0],
                                          xyTry[1], fValue);
     }

     return xyTry[0];

}


double GammaHandler::photoIonization(vector<vector<double>> sourceInfo, vector<double> xyTry) {

	//implement simple delta function to the spectrum
	double fValue = 0.0;
	for(int i = 0; i < sourceInfo.size(); i++) {
		double initialEnergy = sourceInfo[i][0];
		double br = sourceInfo[i][1];
		double pe = sourceInfo[i][2];
		double co = sourceInfo[i][3];
		double pp = sourceInfo[i][4];
		if(abs(xyTry[0]-initialEnergy) < brThresh) {
			fValue = yMax*br*(pe/(pe+co+pp));
		}
	}
	return fValue;

}

double GammaHandler::compton(vector<vector<double>> sourceInfo, vector<double> xyTry) {
	double pi = 3.1415926535897;
	double energyScaleFactor = 511; //mc^2 for electron mass in keV
	double thetaMin = 0.0;
	double thetaMax = pi;
	int simpIterations = 100;
	double simpStep = (thetaMax - thetaMin)/simpIterations;
	double simpCurrentStep = thetaMin;
	double shiftedEnergy, simpResult, kn, B, rY, rPsi, initialEnergy;
	bool draw = true;
	double a = 1.0/137.04;
	double re = pow(0.38616, -12);


	//loop over gamma energies
	for(int i = 0; i < sourceInfo.size(); i++) {
		double initialEnergy = sourceInfo[i][0];
		double br = sourceInfo[i][1];
		double pe = sourceInfo[i][2];
		double co = sourceInfo[i][3];
		double pp = sourceInfo[i][4];
		//get shifted energy with MC
		bool draw = true;
  		while(draw){
    	    rPsi = pi * RandomGen::rndm()->rand_uniform();
    		rY =  10* RandomGen::rndm()->rand_uniform();


    		B = 1.0/(1.0+initialEnergy/energyScaleFactor*(1-cos(rPsi)));
    		kn = pi*pow(B,2)*(B+1.0/B-pow(sin(rPsi),2))*sin(rPsi); //klien nishina
    		if(rY<kn) draw = false;
  		}
  		shiftedEnergy = initialEnergy * (1.0-1.0/(1.0+initialEnergy/energyScaleFactor*(1.0-cos(rPsi)))); //shifted ebergy formula
  		if(abs(xyTry[0]-shiftedEnergy) < brThresh) {
  			return kn*yMax*br*(co/(pe+co+pp));
  		}

		
	}
	return 0.0;

}

double GammaHandler::pairProduction(vector<vector<double>> sourceInfo, vector<double> xyTry) {
	double energyScaleFactor = 511; //mc^2 for electron mass in keV
	double initialEnergy, shiftedEnergy;
	//loop over allowed gamma energies
	for(int i = 0; i < sourceInfo.size(); i++) {
		double initialEnergy = sourceInfo[i][0];
		double br = sourceInfo[i][1];
		double pe = sourceInfo[i][2];
		double co = sourceInfo[i][3];
		double pp = sourceInfo[i][4];
		shiftedEnergy = (0.5)*(initialEnergy - 2*energyScaleFactor); //Pair production energy
		if(abs(xyTry[0]-shiftedEnergy) < brThresh) {
  			return yMax*br*(pp/(pe+co+pp));
  		}


	}
	return 0.0;
}
