#ifndef ABSTRACT_H
#define ABSTRACT_H

class Abstract
{
public:
    enum PrintFormat {
        Short,
        Verbose,
        OnlyId,
        ClassNameAndId
    };

    Abstract(int id = -1);
    virtual ~Abstract();

    int id() { return m_id; }

    // factory method
    static Abstract* createObject() { return 0; }

    virtual void pureVirtual() = 0;
    virtual void unpureVirtual();

    void callPureVirtual();
    void callUnpureVirtual();

    void show(PrintFormat format = Verbose);

protected:
    virtual const char* className() { return "Abstract"; }

private:
    int m_id;
};
#endif // ABSTRACT_H

