#include "auth.h"



namespace NMonitoring {
namespace {
    class TFakeAuthProvider final: public IAuthProvider {
    public:
        TAuthResult Check(const IHttpRequest&) override {
            return TAuthResult::Ok();
        }
    };

} // namespace

std::unique_ptr<IAuthProvider> CreateFakeAuth() {
    return MakeHolder<TFakeAuthProvider>();
}


} // namespace NMonitoring
