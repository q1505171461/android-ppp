#include "include/Adapter/Ifpb/IfpbAdapter.h"
#include "include/Controller/Controller.h"
#include "include/Utils/Com/Com.h"
using namespace std;
using namespace bamboo;
IfpbAdapter::IfpbAdapter()
{
}
IfpbAdapter::~IfpbAdapter()
{
}
void IfpbAdapter::m_inputIfpb(string cprn, t_ifpb &in)
{
    std::unique_lock<std::mutex> lk(m_lock);
    m_ifpbs[cprn] = in;
}
