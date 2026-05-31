class RecursionDetector final
{
    private:
        bool &m_flag;

    public:
        RecursionDetector(bool &flag, const char *type, const char *func)
            : m_flag(flag)
        {
            if(m_flag){
                throw fflpanic("recursion detected in {}::{}", type, func);
            }
            else{
                m_flag = true;
            }
        }

        ~RecursionDetector()
        {
            m_flag = false;
        }
};
