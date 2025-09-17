

class PID{
    public:
        double tau = 0.02, T = 0.02;
        double kP, kI, kD, error;
        double limMin = -1.0, limMax = 1.0, limMinInt = -0.5, limMaxInt = 0.5;
        bool atSetpoint;
        double integrator, prevError, differentiator, prevMeasurement, output;
        double sumError;
        double kSum = 2.0;


        void Reset()
        {
            error = 0.0;
            integrator = 0.0;
            prevError = 0.0;
            differentiator = 0.0;
            prevMeasurement = 0.0;
            atSetpoint = false;
            output = 0.0;
            sumError = 0;
        }
        // void maxSum( double factor ){
        //     kSum = factor;
        // }
        void setPID(double Kp, double Ki, double Kd)
        {
            kP = Kp;
            kI = Ki;
            kD = Kd;
        }
        void setPIDLimits(double LimMin, double LimMax)
        {
            limMin = LimMin; 
            limMax = LimMax; 
        }

        double Calculate( double measurement, double setPoint)
        {
            
            if (setPoint > limMax)    { setPoint = limMax; }
            else if (setPoint < limMin)   { setPoint = limMin; }

            //Error
            error = setPoint - measurement;

            //Proportional
            double proportional = kP * error;

            if ((error == 0) && (prevError == 0) ){//|| error/abs(error) != prevError/abs(prevError)){
                sumError = 0;
            }
            else{
                sumError = sumError + error;

                double maxSum = limMax * kSum;
                if     ( sumError >  maxSum ){ sumError =  maxSum; }
                else if( sumError < -maxSum ){ sumError = -maxSum; }
            }

            integrator = kI * sumError;

            //Integral
            // integrator = integrator + 0.5 * kI * T * (error + prevError);

            if     ( integrator > limMaxInt ){ integrator = limMaxInt; }
            else if( integrator < limMinInt ){ integrator = limMinInt; }
            

            //Band limit derivative
            differentiator = -(2.0 * kD * (measurement - prevMeasurement) + (2.0 * tau - T) * differentiator) / (2.0 * tau + T);

            //Compute
            output = proportional + integrator;

            //Clamp
            if (output > limMax)    { output = limMax; }
            else if (output < limMin)   { output = limMin; }

            //Store variables
            prevError = error;
            prevMeasurement = measurement;

            if ( setPoint == 0 ){
                output = 0;
                sumError = 0;
            }

            // std::cout << "integrator: " << integrator << " proportional: " << proportional << std::endl;
            
            //Return final value
            return output;
        }
};

