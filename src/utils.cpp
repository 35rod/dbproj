#include "utils.hpp"
#include <stdexcept>

namespace io_utils {
    void ensure_good_write(std::ostream& os) {
        if (!os.good()) throw std::ios_base::failure("stream write failed");
    }

    void ensure_good_read(std::istream& is) {
        if (!is.good()) throw std::ios_base::failure("stream read failed");
    }
}