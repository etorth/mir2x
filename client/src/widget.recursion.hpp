class RecursionDetector final
{
    private:
        bool &m_flag;

    public:
        RecursionDetector(bool &flag, const char *type, const char *func)
            : m_flag(flag)
        {
            if(m_flag){
                throw fflerror("recursion detected in %s::%s", type, func);
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
