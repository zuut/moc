@# Copyright (c) 1994-2021.
@# Template for the moc compiler.
@# ///////////////////////////////////////////////
@# //
@# // Used for sample to print out data model
@# //
@# ///////////////////////////////////////////////
@{foreach my-module in ALL_MODULES
@{if exists MODULE 
@{open DataModelFor@(:up1@(MODULE)).txt
MODULE: @(MODULE)

#line @(HEADER_CODE.Lineno) "@(HEADER_CODE.Filename)" 
@(HEADER_CODE.Value)
#line @(OUTPUT_NEXT_LINENO) "@(OUTPUT_FILENAME)" 

#line @(HEADER_END_CODE.Lineno) "@(HEADER_END_CODE.Filename)" 
@(HEADER_END_CODE.Value)
#line @(OUTPUT_NEXT_LINENO) "@(OUTPUT_FILENAME)" 

#line @(SOURCE_CODE.Lineno) "@(SOURCE_CODE.Filename)" 
@(SOURCE_CODE.Value)
#line @(OUTPUT_NEXT_LINENO) "@(OUTPUT_FILENAME)" 
@# ///////////////////////////////////////////////
@# //
@# //  First iterate over all the classes 
@# //  and setup some additional key/value properties
@# //  to use in the rest of the following template patterns.
@# //  
@# ///////////////////////////////////////////////

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//
//
//             CLASSES AND INTERFACES
//
//
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
@{  foreach my-class in ALL_CLASSES
@{    if ( is-from-current-module )
----------------------------------------------------------
----------------------------------------------------------
@@ a line beginning with a single '@@' character.
  Name: @(Name)
  TypeString: @(TypeString)
@{      foreach annotation in annotations
    Annotation: Name: @(Name) , Value: @(Value)
@}      end foreach annotation in annotations
  TypeCode : @(TypeCode)
  Filename: @(Filename)
  Lineno: @(Lineno)
  Type: @(TypeCode)
  IsType: @(IsType)
  IsClass : @(IsClass)
  IsEnum : @(IsEnum)
  IsAggregate : @(IsAggregate)
  IsTypeDef : @(IsTypeDef)
  IsArg : @(IsArg)
  IsMethod : @(IsMethod)
@{      if exists HasParents
  The class @(Name) has the following parents :
    @(:format-list : suffix-sep=',' :foreach parent in parents : @(Name)) .
  The complete list of all super classes and above that this class
  derives is :
    @(:format-list : suffix-sep=',' :foreach parent in all-parents : @(Name)) .
  Immediate SuperClasses and Interfaces:
@{        foreach parent in parents 
    parent @(Name) has the properties:
      Name: @(Name)
      Type.TypeString: @(Type.TypeString)
      TypeCode : @(TypeCode)
      Filename: @(Filename)
      Lineno: @(Lineno)
      IsType: @(IsType)
      IsClass : @(IsClass)
      IsEnum : @(IsEnum)
      IsAggregate : @(IsAggregate)
      IsTypeDef : @(IsTypeDef)
      IsArg : @(IsArg)
      IsMethod : @(IsMethod)

      Type.Name: @(Type.TypeString)
      Type.TypeString: @(Type.TypeString)
      TypeTypeCode: @(Type.TypeCode)
      Type.Filename: @(Type.Filename)
      Type.Header: @(Type.Header)
      Type.Lineno: @(Type.Lineno)
      Type.IsType: @(Type.IsType)
      Type.IsClass : @(Type.IsClass)
      Type.IsEnum: @(Type.IsEnum)
      Type.IsAggregate: @(Type.IsAggregate)
      Type.IsTypeDef: @(Type.IsTypeDef)
      Type.IsArg : @(Type.IsArg)
      Type.IsMethod : @(Type.IsMethod)
@{          if empty properties
  No Properties for @(Name)
@|          else
  Properties of @(Name)
@}          end if empty properties
@{          foreach property in properties
      Name: @(Name)
      Type.TypeString: @(Type.TypeString)
      TypeCode: @(TypeCode)
      Filename: @(Filename)
      Lineno: @(Lineno)
      IsType: @(IsType)
      IsClass : @(IsClass)
      IsEnum : @(IsEnum)
      IsAggregate : @(IsAggregate)
      IsTypeDef : @(IsTypeDef)
      ArrayLength: @(:? ArrayLength)
      IsArg : @(IsArg)
      IsMethod : @(IsMethod)

      Type.Name: @(Type.Name)
      Type.TypeString: @(Type.TypeString)
      TypeTypeCode: @(Type.TypeCode)
      Type.Filename: @(Type.Filename)
      Type.Header: @(Type.Header)
      Type.Lineno: @(Type.Lineno)
      Type.IsType: @(Type.IsType)
      Type.IsClass : @(Type.IsClass)
      Type.IsEnum: @(Type.IsEnum)
      Type.IsAggregate: @(Type.IsAggregate)
      Type.IsTypeDef: @(Type.IsTypeDef)
      Type.IsArg : @(Type.IsArg)
      Type.IsMethod : @(Type.IsMethod)
@{            if ! last
     ...............
@}            end if ! last
@}          end foreach property in properties
@{          if empty methods
  No methods.
@|          else
  Methods:
@}          end if empty methods
@{          foreach method in methods
      Belongs to: @(parent::Name) used by @(my-class::Name)
      Name: @(Name)
      TypeCode: @(TypeCode)
      Filename: @(Filename)
      Lineno: @(Lineno)
      IsType: @(IsType)
      IsClass : @(IsClass)
      IsEnum : @(IsEnum)
      IsAggregate : @(IsAggregate)
      IsTypeDef : @(IsTypeDef)
      IsArg : @(IsArg)
      IsMethod : @(IsMethod)
@}          end foreach method in methods
@}        end foreach parent in parents 
@|      else
    This class does not derive from any other class or interface.
@}      end if exists HasParents

@{      if empty properties
  No Properties for @(Name)
@|      else
  Properties of @(Name)
@}      end if empty properties
@{      foreach property in properties
    Name: @(Name)
    code : @(code) 
    Type.TypeString: @(Type.TypeString)
    TypeCode : @(TypeCode)
    Filename: @(Filename)
    Lineno: @(Lineno)
    IsType: @(IsType)
    IsClass : @(IsClass)
    IsEnum : @(IsEnum)
    IsAggregate : @(IsAggregate)
    IsTypeDef : @(IsTypeDef)
    ArrayLength: @(:? ArrayLength)
    IsArg : @(IsArg)
    IsMethod : @(IsMethod)

    Type.Name: @(Type.Name)
    Type.TypeString: @(Type.TypeString)
    TypeTypeCode: @(Type.TypeCode)
    Type.Filename: @(Type.Filename)
    Type.Header: @(Type.Header)
    Type.Lineno: @(Type.Lineno)
    Type.IsType: @(Type.IsType)
    Type.IsClass : @(Type.IsClass)
    Type.IsEnum: @(Type.IsEnum)
    Type.IsAggregate: @(Type.IsAggregate)
    Type.IsTypeDef: @(Type.IsTypeDef)
    Type.IsArg : @(Type.IsArg)
    Type.IsMethod : @(Type.IsMethod)

  @@(:length annotations)  = @(:length annotations) 
  @@(:nth 0 annotations Name)  = @(:nth 0 annotations  Name)
  @@(:nth 0 annotations Value)  = @(:nth 0 annotations  Value)
  @@(:nth 1 annotations Name)  = @(:nth 1 annotations  Name)
  @@(:nth 1 annotations Value)  = @(:nth 1 annotations  Value)
@{        foreach annotation in annotations
      Annotation: Name: @(Name) , Value: @(Value)
         @@(:up1 @@(Name)) = get@(:up1 @(Name))
  @@(:up1 PascalCaseWordList)  = @(:up1 PascalCaseWordList) 
  @@(:up1 camelCaseWordList) = @(:up1 camelCaseWordList) 
  @@(:up1 snake_case_word_list) = @(:up1 snake_case_word_list) 

         @@(:uppercase-all @@(Name)) = @(:uppercase-all @(Name))
  @@(:uppercase-all PascalCaseWordList)  = @(:uppercase-all PascalCaseWordList) 
  @@(:uppercase-all camelCaseWordList) = @(:uppercase-all camelCaseWordList) 
  @@(:uppercase-all snake_case_word_list) = @(:uppercase-all snake_case_word_list) 
         @@(:uppercase-prefix @@(Name)) = @(:uppercase-prefix @(Name))
  @@(:uppercase-prefix PascalCaseWordList)  = @(:uppercase-prefix PascalCaseWordList) 
  @@(:uppercase-prefix PAscalCaseWordList)  = @(:uppercase-prefix PAscalCaseWordList) 
  @@(:uppercase-prefix camelCaseWordList) = @(:uppercase-prefix camelCaseWordList) 
  @@(:uppercase-prefix @@(:up1 camelCaseWordList)) = @(:uppercase-prefix @(:up1 camelCaseWordList)) 
  @@(:uppercase-prefix snake_case_word_list) = @(:uppercase-prefix snake_case_word_list) 
         @@(:lowercase-all @@(Name)) = @(:lowercase-all @(Name))
  @@(:lowercase-all PascalCaseWordList)  = @(:lowercase-all PascalCaseWordList) 
  @@(:lowercase-all camelCaseWordList) = @(:lowercase-all camelCaseWordList) 
  @@(:lowercase-all snake_case_word_list) = @(:lowercase-all snake_case_word_list) 


-----
  @@(:PascalCase PascalCaseWordList)  = @(:PascalCase PascalCaseWordList) 
  @@(:PascalCase camelCaseWordList) = @(:PascalCase camelCaseWordList) 
  @@(:PascalCase snake_case_word_list) = @(:PascalCase snake_case_word_list) 

-----
  @@(:camelCase PascalCaseWordList)  = @(:camelCase PascalCaseWordList) 
  @@(:camelCase camelCaseWordList) = @(:camelCase camelCaseWordList) 
  @@(:camelCase snake_case_word_list) = @(:camelCase snake_case_word_list) 

-----
  @@(:snake-case PascalCaseWordList)  = @(:snake-case PascalCaseWordList) 
  @@(:snake-case camelCaseWordList) = @(:snake-case camelCaseWordList) 
  @@(:snake-case snake_case_word_list) = @(:snake-case snake_case_word_list) 


@}        end foreach annotation in annotations
@{        if ! last property
     ...............
@}        end if ! last property
@}      end foreach property in properties
@{      if empty methods
  No methods.
@|      else
  Methods:
@}      end if empty methods
@{      foreach method in methods
    .....
    Belongs to: @(my-class::Name)
    Name: @(Name)
@{        foreach annotation in annotations
      Annotation: Name: @(Name) , Value: @(Value)
@}        end foreach annotation in annotations
    ReturnType: @(ReturnType)
    ReturnType.Type.IsClass: @(ReturnType.Type.IsClass)
    ReturnType.Type.IsType: @(ReturnType.Type.IsType)
    ReturnType.Type.IsEnum: @(ReturnType.Type.IsEnum)
    ReturnType.Type.IsAggregate: @(ReturnType.Type.IsAggregate)
    ReturnType.Type.IsTypeDef: @(ReturnType.Type.IsTypeDef)
    ReturnType.Type.Filename: @(ReturnType.Type.Filename)
    ReturnType.Type.Lineno: @(ReturnType.Type.Lineno)
    TypeCode: @(TypeCode)
    Filename: @(Filename)
    Lineno: @(Lineno)
    IsType: @(IsType)
    IsClass : @(IsClass)
    IsEnum : @(IsEnum)
    IsAggregate : @(IsAggregate)
    IsTypeDef : @(IsTypeDef)
    IsArg : @(IsArg)
    IsMethod : @(IsMethod)
    All Args Boo: '@(:format-list : suffix-sep=','     @\
        :foreach a in in-arguments                     @\
          :in @(Type.TypeString) the@(Name):           @\
        foreach a in out-arguments                     @\
            : out @(Type.TypeString) the@(Name):       @\
        foreach a in error-arguments                   @\
            : err @(Type.TypeString) the@(Name)) 'done.

    In Arguments to Method:
@{        foreach in-arg in in-arguments
      Name: @(Name)
      TypeString: @(TypeString)
      Type.TypeString: @(Type.TypeString)
      Method:  @(method::Name) 
      TypeCode : @(TypeCode)
      Filename: @(Filename)
      Lineno: @(Lineno)
      IsType: @(IsType)
      IsClass : @(IsClass)
      IsEnum : @(IsEnum)
      IsAggregate : @(IsAggregate)
      IsTypeDef : @(IsTypeDef)
      IsArg : @(IsArg)
      IsMethod : @(IsMethod)
      Type.Name: @(Type.Name)
      Type.TypeString: @(Type.TypeString)
      Type.Filename: @(Type.Filename)
      Type.Header: @(:? Type.Header : N/A)
      Type.Lineno: @(Type.Lineno)
      Type.IsType: @(Type.IsType)
      Type.IsClass : @(Type.IsClass)
      Type.IsEnum: @(Type.IsEnum)
      Type.IsAggregate: @(Type.IsAggregate)
      Type.IsTypeDef: @(Type.IsTypeDef)
      Type.IsArg : @(Type.IsArg)
      Type.IsMethod : @(Type.IsMethod)
@{          foreach annotation in annotations
        Annotation: Name: @(Name) Value: @(Value)
          TypeCode : @(TypeCode)
          Filename: @(Filename)
          Lineno: @(Lineno)
          IsType: @(IsType)
          IsClass : @(IsClass)
          IsEnum : @(IsEnum)
          IsAggregate : @(IsAggregate)
          IsTypeDef : @(IsTypeDef)
          IsArg : @(IsArg)
          IsMethod : @(IsMethod)
@}          end foreach annotation in annotations

@}        end foreach in-arg in in-arguments
    Error Arguments to Method:
@{        foreach err-arg in error-arguments
      Name: @(Name)
      TypeString: @(TypeString)
      Type.TypeString: @(Type.TypeString)
      Method:  @(method::Name) 
      TypeCode : @(TypeCode)
      Filename: @(Filename)
      Lineno: @(Lineno)
      IsType: @(IsType)
      IsClass : @(IsClass)
      IsEnum : @(IsEnum)
      IsAggregate : @(IsAggregate)
      IsTypeDef : @(IsTypeDef)

      Type.TypeString: @(Type.TypeString)
      TypeTypeCode: @(:? Type.TypeCode: N/A)
      Type.Filename: @(Type.Filename)
      Type.Header: @(:? Type.Header:N/A)
      Type.Lineno: @(Type.Lineno)
      Type.IsType: @(Type.IsType)
      Type.IsClass : @(Type.IsClass)
      Type.IsEnum: @(Type.IsEnum)
      Type.IsAggregate: @(Type.IsAggregate)
      Type.IsTypeDef: @(Type.IsTypeDef)
@{          foreach annotation in annotations
        Annotation: Name: @(Name) Value: @(Value)
          TypeString: @(TypeString)
          TypeCode: @(TypeCode)
          Filename: @(Filename)
          Header: @(Header)
          Lineno: @(Lineno)
          IsType: @(IsType)
          IsClass : @(IsClass)
          IsEnum: @(IsEnum)
          IsAggregate: @(IsAggregate)
          IsTypeDef: @(IsTypeDef)
@}          end foreach annotation in annotations

@}        end foreach err-arg in error-arguments
@}      end foreach method in methods


@}    end if ( is-from-current-module )
@}  end foreach my-class in ALL_CLASSES

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//
//
//             TYPES, TYPEDEFS, STRUCTS and ENUMS 
//
//
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
@{  foreach my-types in ALL_TYPES
@{    if ( is-from-current-module )
  Name: @(Name)
  TypeString: @(TypeString)
  TypeCode: @(:? TypeCode)
@{      foreach annotation in annotations
    Annotation: Name: @(Name) , Value: @(Value)
@}      end foreach annotation in annotations
  Filename: @(Filename)
  Header: @(Header)
  Lineno: @(Lineno)
  IsType: @(IsType)
  IsClass : @(IsClass)
  IsEnum: @(IsEnum)
  IsAggregate: @(IsAggregate)
  IsTypeDef: @(IsTypeDef)
@{      if @(IsTypeDef)
  TypeDefString: @(TypeDefString)
    TypeString: @(TypeDefString.TypeString)
    TypeCode: @(:? TypeDefString.TypeCode)
    Filename: @(TypeDefString.Filename)
    Header: @(:? TypeDefString.Header)
    Lineno: @(TypeDefString.Lineno)
    IsType: @(TypeDefString.IsType)
    IsClass : @(TypeDefString.IsClass)
    IsEnum: @(TypeDefString.IsEnum)
    IsAggregate: @(TypeDefString.IsAggregate)
    IsTypeDef: @(TypeDefString.IsTypeDef)
@}      end if @(IsTypeDef)
@{      if @(IsAggregate)
  Properties of @(Name)
@{        foreach property in properties
    Name: @(Name)
    Type.TypeString: @(Type.TypeString)
    Type.Filename: @(Type.Filename)
    Type.Header: @(Type.Header)
    Type.Lineno: @(Type.Lineno)
    Type.IsType: @(Type.IsType)
    Type.IsClass : @(Type.IsClass)
    Type.IsEnum: @(Type.IsEnum)
    Type.IsAggregate: @(Type.IsAggregate)
    Type.IsTypeDef: @(Type.IsTypeDef)

    Filename: @(Filename)
    Lineno: @(Lineno)
    TypeCode : @(:if exists TypeCode :then @(TypeCode) :else N/A)
    ArrayLength: @(:? ArrayLength)
@{          foreach annotation in annotations
      Annotation: Name: @(Name) , Value: @(Value)
@}          end foreach annotation in annotations
@{          if ! last property
     ...............
@}          end if ! last property
@}        end foreach property in properties
@|      else
  Not an aggregate.
@}      end if @(IsAggregate)
@{      if @(IsEnum)
  Enum:
@{        foreach entry in enumentries
    Enum Entry: Name: @(Name) , Value: @(Value)
@}        end foreach entry in enumentries
@|      else
  Not an enum.
@}      end if @(IsEnum)

@}    end if ( is-from-current-module )
@}  end foreach my-types in ALL_TYPES



////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//
//
//             CONSTANTS
//
//
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
@{  foreach constant in ALL_CONSTANTS
@{    if ( is-from-current-module )
  Name: @(Name)
  Value: @(Value)
@{      foreach annotation in annotations
    Annotation: Name: @(Name) , Value: @(Value)
@}      end foreach annotation in annotations
  Filename: @(Filename)
  Lineno: @(Lineno)


@}    end if ( is-from-current-module )
@}  end foreach constant in ALL_CONSTANTS

----------------------------------------------------------
----------------------------------------------------------


@}end open DataModel.txt
@}endif exists MODULE
@}end foreach my-module in ALL_MODULES

