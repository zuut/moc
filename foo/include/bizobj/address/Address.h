/*
 *  COPYRIGHT (C) 2000-2021 ZUUT, INC. ALL RIGHTS RESERVED.
 */
#ifndef BIZOBJ_ADDRESS
#define BIZOBJ_ADDRESS

#line 14 "test_app/test_app.moc" 




   static char header_code= "   typically this is code inserted at the beginning of a header file.   Comments are removed.";



#line 17 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/address/Address.h" 

#include <memory>
#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
/**
 * <P>
 * Address
 * a Address.
 *    An address of some real world entity ;
 * </P>
 *
 * @version $Id$
 */
class Address : public BizObject  {
public:
private:    
    /**The street1 property - The street. */
    private String street1;
    /**The street2 property - 2nd line of street info. */
    private String street2;
    /**The city property - The City. */
    private String city;
    /**The state property - The State. */
    private String state;
    /**The zipcode property - The Zip Code. */
    private String zipcode;
    /**The CountryId property - The Country. */
    private Long CountryId;
};

#line 127 "test_app/test_app.moc" 


   static char header_end_code= "   typically this is code inserted at the end of a header file.";


#line 56 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/address/Address.h" 

#endif // BIZOBJ_ADDRESS
