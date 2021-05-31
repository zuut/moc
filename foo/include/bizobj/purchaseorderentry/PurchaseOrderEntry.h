/*
 *  COPYRIGHT (C) 2000-2021 ZUUT, INC. ALL RIGHTS RESERVED.
 */
#ifndef BIZOBJ_PURCHASEORDERENTRY
#define BIZOBJ_PURCHASEORDERENTRY

#line 14 "test_app/test_app.moc" 




   static char header_code= "   typically this is code inserted at the beginning of a header file.   Comments are removed.";



#line 17 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/purchaseorderentry/PurchaseOrderEntry.h" 

#include <memory>
#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
/**
 * <P>
 * PurchaseOrderEntry
 * a PurchaseOrderEntry.
 *   Associates a particular item with a purchase order;
 * </P>
 *
 * @version $Id$
 */
class PurchaseOrderEntry : public BizObject  {
public:
private:    
    /**The itemId property - The item that was ordered. */
    private Long itemId;
    /**The purchaseOrderId property - The purchase order. */
    private Long purchaseOrderId;
};

#line 127 "test_app/test_app.moc" 


   static char header_end_code= "   typically this is code inserted at the end of a header file.";


#line 48 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/purchaseorderentry/PurchaseOrderEntry.h" 

#endif // BIZOBJ_PURCHASEORDERENTRY
