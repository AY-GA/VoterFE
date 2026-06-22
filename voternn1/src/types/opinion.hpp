#ifndef VOTERNN_TYPE_OPINION
#define VOTERNN_TYPE_OPINION
#include "../util/math.hpp"

namespace voternn {
	/// @brief opinion_position class describes positions of each voter/party/MP.
	struct opinion_position {
	public:
		double values[12] {};
		
		double& security = this->values[0];
		double& risk_aversion = this->values[1];
		double& public_spending = this->values[2];
		double& cost_of_living = this->values[3];
		double& minorities_support = this->values[4];
		double& stability = this->values[5];
		double& job_security = this->values[6];
		double& future_outlook = this->values[7];
		double& equality_markets = this->values[8];
		double& nation_globe = this->values[9];
		double& liberty_authority = this->values[10];
		double& tradition_progress = this->values[11];
	};

	///
	///@param[in] pos1 Positions of first voter/party/MP
	///@param[in] pos2 Positions of second voter/party/MP
	///@returns Distance between position vectors.
	///
	double calculate_opinion_distance(
		const opinion_position& pos1,
		const opinion_position& pos2
	);

} //namespace voternn

#endif
