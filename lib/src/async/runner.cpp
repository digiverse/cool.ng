
#include "cool/ng/async/runner.h"
#include "lib/async/executor.h"

namespace cool { namespace ng { namespace async {

runner::runner(RunPolicy policy_)
{
  m_impl = std::make_shared<impl::executor>(policy_);
}

runner::~runner()
{ /* noop */ }

const std::string& runner::name() const
{
  return m_impl->name();
}

const std::shared_ptr<impl::executor>& runner::impl() const
{
  return m_impl;
}

void runner::start()
{
  m_impl->start();
}

void runner::stop()
{
  m_impl->stop();
}


} } } // namespace
