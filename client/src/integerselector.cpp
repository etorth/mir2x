#include "integerselector.hpp"

IntegerSelector::IntegerSelector(IntegerSelector::InitArgs args)
    : ValueSelector
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),
          .h = std::move(args.h),

          .input
          {
              .w = std::move(args.input.w),
              .font = std::move(args.input.font),
              .cursor = std::move(args.input.cursor),
              .onChange = std::move(args.input.onChange),
              .validate = std::move(args.input.validate),
          },

          .button
          {
              .w = std::move(args.button.w),
          },

          .upTrigger = [r = args.range, tgr = std::move(args.upTrigger), this](int clicks)
          {
              const auto [_, max] = Widget::evalGetter(r, this);
              const auto val = std::stoll(getValue());

              if(val < max){
                  setValue(std::to_string(val + 1).c_str());
                  Button::evalTriggerCBFunc(tgr, this, clicks);
              }
              else if(val > max){
                  setValue(std::to_string(max).c_str());
                  Button::evalTriggerCBFunc(tgr, this, clicks);
              }
          },

          .downTrigger = [r = args.range, tgr = std::move(args.downTrigger), this](int clicks)
          {
              const auto [min, _] = Widget::evalGetter(r, this);
              const auto val = std::stoll(getValue());

              if(val > min){
                  setValue(std::to_string(val - 1).c_str());
                  Button::evalTriggerCBFunc(tgr, this, clicks);
              }
              else if(val < min){
                  setValue(std::to_string(min).c_str());
                  Button::evalTriggerCBFunc(tgr, this, clicks);
              }
          },

          .parent = std::move(args.parent),
      }}
{
    setValue(std::to_string(Widget::evalGetter(args.range, this).first).c_str());
}

std::optional<int> IntegerSelector::getInt() const
{
    if(const auto s = getValue(); s.empty()){
        return std::nullopt;
    }
    else{
        try{
            return std::stoi(s);
        }
        catch(...){
            throw fflerror("invalid integer string: %s", to_cstr(s));
        }
    }
}
