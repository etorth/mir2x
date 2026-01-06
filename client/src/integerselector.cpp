#include "integerselector.hpp"

IntegerSelector::IntegerSelector(IntegerSelector::InitArgs args)
    : ValueSelector
      {{
          .validate = [this]
          {
              try{}catch(...){
                  return false;
              }
              return true;
          },

          .upTrigger = [this](int)
          {
              const auto [_, max] = Widget::evalGetter(args.range, this);
              const auto val = std::stoll(m_input.getRawString());

              if(val < max){
                  m_input.setInput(std::to_string(val + 1).c_str());
              }
              else if(val > max){
                  m_input.setInput(std::to_string(max).c_str());
              }
          },

          .downTrigger = [](int)
          {
              const auto [min, _] = Widget::evalGetter(args.range, this);
              const auto val = std::stoll(m_input.getRawString());

              if(val > min){
                    m_input.setInput(std::to_string(val - 1).c_str());
                }
                else if(val < min){
                    m_input.setInput(std::to_string(min).c_str());
              }
          },
      }}
{}
