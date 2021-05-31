/*
 *  COPYRIGHT (C) 2000-2021 ZUUT, INC. ALL RIGHTS RESERVED.
 */
#ifndef BIZOBJ_ITEM
#define BIZOBJ_ITEM

#line 14 "test_app/test_app.moc" 




   static char header_code= "   typically this is code inserted at the beginning of a header file.   Comments are removed.";



#line 17 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/item/Item.h" 

#include <memory>
#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
/**
 * <P>
 * Item
 * a Item.
 *    A specific inventory item belonging to a product SKU for sell ;
 * </P>
 *
 * @version $Id$
 */
class Item : public BizObject  {
public:
private:    
    /**The description property - Customer's description of the product item. */
    private String description;
    /**The productId property - The associated product for this item. */
    private Long productId;
};

#line 127 "test_app/test_app.moc" 


   static char header_end_code= "   typically this is code inserted at the end of a header file.";


#line 48 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/item/Item.h" 

#endif // BIZOBJ_ITEM
