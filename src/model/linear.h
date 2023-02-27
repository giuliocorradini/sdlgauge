#include "model.h"

class LinearModel: public Model {
    private:
        int power;
        int revs;
    
    public:
        LinearModel(int power);

        int get_revs() override;

        void rev_up();

        void rev_down();
};