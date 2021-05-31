/*
 *  COPYRIGHT (C) 2000-2021 ZUUT, INC. ALL RIGHTS RESERVED.
 */
#ifndef BIZOBJ_PRODUCT
#define BIZOBJ_PRODUCT

#line 14 "test_app/test_app.moc" 




   static char header_code= "   typically this is code inserted at the beginning of a header file.   Comments are removed.";



#line 17 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/product/Product.h" 

#include <memory>
#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
/**
 * <P>
 * Product
 * a Product.
 *    A product SKU for sell ;
 * </P>
 *
 * @version $Id$
 */
class Product : public BizObject  {
public:
private:    
    /**The description property - Customer's description of the product. */
    private String description;
};

#line 127 "test_app/test_app.moc" 


   static char header_end_code= "   typically this is code inserted at the end of a header file.";


#line 46 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/product/Product.h" 

#endif // BIZOBJ_PRODUCT
