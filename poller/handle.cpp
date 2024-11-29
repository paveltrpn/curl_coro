
#include "handle.h"

namespace poller {

Handle::Handle() {
    handle_ = curl_easy_init();
}

}  // namespace poller
