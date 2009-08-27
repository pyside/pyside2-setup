#ifndef REFERENCE_H
#define REFERENCE_H

class Reference
{
public:
    explicit Reference(int objId = -1)
            : m_objId(objId) {}
    ~Reference() {}

    double objId() { return m_objId; }
    void setObjId(int objId) { m_objId = objId; }

    static int usesReference(Reference& r) { return r.m_objId; }
    static int usesConstReference(const Reference& r) { return r.m_objId; }

    virtual int usesReferenceVirtual(Reference& r, int inc);
    virtual int usesConstReferenceVirtual(const Reference& r, int inc);

    int callUsesReferenceVirtual(Reference& r, int inc);
    int callUsesConstReferenceVirtual(const Reference& r, int inc);

    void show() const;

private:
    int m_objId;
};

#endif // REFERENCE_H

