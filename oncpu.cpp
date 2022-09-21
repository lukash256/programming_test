#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <random>


class Agent {
    const double Brand_Factor;
    uint64_t curLineNum;
	std::string headLine;
	
	enum class State {Norm, Lost, Gained, Regained} state = State::Norm;
    enum class Breed {C, NC};
    struct Presentation {
        Breed Agent_Breed;
        double Policy_ID;
        uint16_t Payment_at_Purchase, Age, Social_Grade, Inertia_for_Switch;
        float Attribute_Brand, Attribute_Price, Attribute_Promotions;
        bool Auto_Renew;
    };
    
    std::unique_ptr<Presentation> pBreed;
    
    void chk() const {
        if(!pBreed)
            throw std::runtime_error("No csv data loaded");
    }
    
    double random() const {
		std::random_device rd;
		std::default_random_engine eng(rd());
		std::uniform_real_distribution<double> distr(-10, 10);
		
		return distr(eng);
	}
  
  public:
    Agent(const double& Brand_Factor) 
        : Brand_Factor(Brand_Factor) 
    {
        if(Brand_Factor < 0.1 || Brand_Factor > 2.9)
            throw std::runtime_error("Brand_Factor is out of range [0.1..2.9]");
    }
    
    Agent& incAge(uint64_t years = 1) {
        chk();
        
        for(uint64_t i = 0; i < years; ++i)	{
			++pBreed->Age;
			if(!pBreed->Auto_Renew) {
				srand(time(NULL));
				double rand =  /*static_cast<double>(std::rand() - std::rand()) / RAND_MAX*/ random() * 3;
				auto Affinity = pBreed->Payment_at_Purchase/pBreed->Attribute_Price + (rand * pBreed->Attribute_Promotions * pBreed->Inertia_for_Switch);
				if(pBreed->Agent_Breed == Breed::C && Affinity < (pBreed->Social_Grade * pBreed->Attribute_Brand)) {
					pBreed->Agent_Breed = Breed::NC;
					if(state == State::Norm)
						state = State::Lost;
					else if(state == State::Regained)
						state = State::Lost;
					else if(state == State::Gained)
						state = State::Norm;
				}
				else if (pBreed->Agent_Breed == Breed::NC && Affinity < (pBreed->Social_Grade * pBreed->Attribute_Brand * Brand_Factor)) {
					pBreed->Agent_Breed = Breed::C;
					if(state == State::Norm)
						state = State::Gained;
					else if(state == State::Lost)
						state = State::Regained;
				}
			}
		}
        
        return *this;
    }
    
    bool isLost() const	{		return state == State::Lost;	}
	bool isGained() const {		return state == State::Gained;	}
	bool isRegained() const {	return state == State::Regained;}
	
	bool isBreed_C() const  {	
		chk(); 
		return pBreed->Agent_Breed == Breed::C; 
	}
    
    friend std::ostream& operator<<(std::ostream& out, Agent& agent) {
		if(out.tellp() == 0)
			out << agent.headLine;
		
		std::string breed = (agent.pBreed->Agent_Breed == Breed::C) ? "Breed_C," : "Breed_NC,";
		out << std::fixed << std::setprecision(1)
			<< char(0x0D) << breed
			<< agent.pBreed->Policy_ID << ','
			<< agent.pBreed->Age << ','
			<< agent.pBreed->Social_Grade << ','
			<< agent.pBreed->Payment_at_Purchase << ','
			<< agent.pBreed->Attribute_Brand << ','
			<< agent.pBreed->Attribute_Price << ','
			<< agent.pBreed->Attribute_Promotions << ','
			<< agent.pBreed->Auto_Renew << ','
			<< agent.pBreed->Inertia_for_Switch;
			
			return out;
	}
    
    friend std::istream& operator>>(std::istream& in, Agent& agent) {
		agent.state = State::Norm;
		if(agent.curLineNum == 0)
			std::getline(in, agent.headLine, char(0x0D));

        std::string line;
        std::getline(in, line, char(0x0D));
		if(line.empty())
          return in;
        
		std::stringstream lineStream(line);
		std::string cell;
		std::unique_ptr<Presentation> pBreed (new Presentation);
		
		// Agent_Breed
		std::getline(lineStream, cell, ',');
		pBreed->Agent_Breed = (cell == "Breed_C") ? Breed::C : Breed::NC;
		
		// Policy_ID
		std::getline(lineStream, cell, ',');
		pBreed->Policy_ID = std::stod(cell);
		
		// Age
		std::getline(lineStream, cell, ',');
		pBreed->Age = std::stoul(cell);

		// Social_Grade
		std::getline(lineStream, cell, ',');
		pBreed->Social_Grade = std::stoul(cell);
		
		// Payment_at_Purchase
		std::getline(lineStream, cell, ',');
		pBreed->Payment_at_Purchase = std::stoul(cell);
		
		// Attribute_Brand
		std::getline(lineStream, cell, ',');
		pBreed->Attribute_Brand = std::stof(cell);
		
		// Attribute_Price
		std::getline(lineStream, cell, ',');
		pBreed->Attribute_Price = std::stof(cell);
		
		// Attribute_Promotions
		std::getline(lineStream, cell, ',');
		pBreed->Attribute_Promotions = std::stof(cell);
		
		// Auto_Renew
		std::getline(lineStream, cell, ',');
		pBreed->Auto_Renew = (cell == "0") ? false : true;
		
		// Inertia_for_Switch
		std::getline(lineStream, cell, ',');
		pBreed->Inertia_for_Switch = std::stoul(cell);
		
		agent.pBreed.reset(pBreed.release());
		
		++agent.curLineNum;
        
        return in;
    }
};

const int years = 15;

int main (int argc, char* argv[]) {
	
	if(argc < 2)
		throw std::runtime_error("Brand_Factor is not provided");
	
	Agent agent(atof(argv[1]));
	
	std::ifstream inFile  ("PseudoData.csv");
	
	std::vector<std::ofstream*> outFilesVP;
	for(int i = 1; i <= years; ++i)
		outFilesVP.push_back(new std::ofstream(std::string("PseudoDataYear") + std::to_string(i) + ".csv", std::ofstream::trunc));
	
	std::ofstream ofBreed_C ("Breed_C.csv", std::ofstream::trunc);
	std::ofstream ofBreed_NC("Breed_NC.csv", std::ofstream::trunc);
	
	std::ofstream ofLost ("Lost.csv", std::ofstream::trunc);
	std::ofstream ofGained("Gained.csv", std::ofstream::trunc);
	std::ofstream ofRegained("Regained.csv", std::ofstream::trunc);
	
	while(inFile >> agent) {
		for(auto& f : outFilesVP) {
			agent.incAge();
			*f << agent;
		}
		
		if(agent.isBreed_C())
			ofBreed_C << agent;
		else
			ofBreed_NC << agent;
		
		if(agent.isLost())
			ofLost << agent;
		else if(agent.isGained())
			ofGained << agent;
		else if(agent.isRegained())
			ofRegained << agent;
	}
	
	ofRegained.close();
	ofGained.close();
	ofLost.close();
	
	ofBreed_NC.close();
	ofBreed_C.close();
	
	for(auto& f : outFilesVP){
		f->close();
		delete f;
	}
	
	inFile.close();

	return 0;
}
