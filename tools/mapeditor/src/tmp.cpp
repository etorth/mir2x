// uint32_t nInput
{
    if(m_Eq->value()){
        return nInput == Mask();
    }

    if(m_Neq->value()){
        return nInput != Mask();
    }

    if(m_Overlap->value()){
        return ((nInput & Mask()) != 0);
    }

    if(m_Exclude->value()){
        return ((nInput & Mask()) == 0);
    }

    if(m_Subset->value()){
        return false
            || (nInput == 0)
            || (((nInput & Mask()) != 0) && ((nInput & (~Mask())) == 0));
    }

    if(m_Superset->value()){
        return false
            || (Mask() == 0)
            || (((nInput & Mask()) != 0) && ((nInput & (~Mask())) != 0));
    }

    if(m_Complement->value()){
        return nInput == (~Mask());
    }

    return false;
}
