/*
 *  COPYRIGHT (C) 2000-2021 ZUUT, INC. ALL RIGHTS RESERVED.
 */
#ifndef BIZOBJ_PURCHASEORDER
#define BIZOBJ_PURCHASEORDER

#line 14 "test_app/test_app.moc" 




   static char header_code= "   typically this is code inserted at the beginning of a header file.   Comments are removed.";



#line 17 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/purchaseorder/PurchaseOrder.h" 

#include <memory>
#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
/**
 * <P>
 * PurchaseOrder
 * a PurchaseOrder.
 *   An order to purchase 1 or more items.;
 * </P>
 *
 * @version $Id$
 */
class PurchaseOrder : public BizObject  {
public:
private:    
    /**The orderDate property - The date the customer placed the order. */
    private Date orderDate;
    /**The total property - The total for the order. */
    private Float total;
    /**The customerId property - The customer that made the purchase. */
    private Long customerId;
    /**The deliveryAddressId property - the delivery address. */
    private Long deliveryAddressId;
    /**The creditCardId property - the credit card. */
    private Long creditCardId;
};

#line 127 "test_app/test_app.moc" 


   static char header_end_code= "   typically this is code inserted at the end of a header file.";


#line 54 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/purchaseorder/PurchaseOrder.h" 

#endif // BIZOBJ_PURCHASEORDER
