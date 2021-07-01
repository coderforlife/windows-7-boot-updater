/*
 * Windows 7 Boot Updater (github.com/coderforlife/windows-7-boot-updater)
 * Copyright (C) 2021  Jeffrey Bush - Coder for Life
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#using <System.Management.dll>

namespace ROOT {
    namespace WMI {

	using namespace System::ComponentModel;
	using namespace System::Management;
    
    // Functions ShouldSerialize<PropertyName> are functions used by VS property browser to check if a particular property has to be serialized. These functions are added for all ValueType properties ( properties of type Int32, BOOL etc.. which cannot be set to null). These functions use Is<PropertyName>Null function. These functions are also used in the TypeConverter implementation for the properties to check for NULL value of property so that an empty value can be shown in Property browser in case of Drag and Drop in Visual studio.
    // Functions Is<PropertyName>Null() are used to check if a property is NULL.
    // Functions Reset<PropertyName> are added for Nullable Read/Write properties. These functions are used by VS designer in property browser to set a property to NULL.
    // Every property added to the class for WMI property has attributes set to define its behavior in Visual Studio designer and also to define a TypeConverter to be used.
    // An Early Bound class generated for the WMI class.BcdObject
    ref class BcdObject sealed : public System::ComponentModel::Component {
        public : ref class BcdObjectCollection;
        //public : ref class WMIValueTypeConverter;
        public : ref class ManagementSystemProperties;
        
        // Private property to hold the WMI namespace in which the class resides.
        private: static string  CreatedWmiNamespace = L"root\\WMI";
        
        // Private property to hold the name of WMI class which created this class.
        private: static string  CreatedClassName = L"BcdObject";
        
        // Private member variable to hold the ManagementScope which is used by the various methods.
        private: static System::Management::ManagementScope^  statMgmtScope = nullptr;
        
        private: ROOT::WMI::BcdObject::ManagementSystemProperties^  PrivateSystemProperties;
        
        // Underlying lateBound WMI object.
        private: System::Management::ManagementObject^  PrivateLateBoundObject;
        
        // Member variable to store the 'automatic commit' behavior for the class.
        private: bool AutoCommitProp;
        
        // Private variable to hold the embedded property representing the instance.
        private: System::Management::ManagementBaseObject^  embeddedObj;
        
        // The current WMI object used
        private: System::Management::ManagementBaseObject^  curObj;
        
        // Flag to indicate if the instance is an embedded object.
        private: bool isEmbedded;
        
        // Below are different overloads of constructors to initialize an instance of the class with a WMI object.
        public: BcdObject();
        public: BcdObject(string  keyId, string  keyStoreFilePath);
        public: BcdObject(System::Management::ManagementScope^  mgmtScope, string  keyId, string  keyStoreFilePath);
        public: BcdObject(System::Management::ManagementPath^  path, System::Management::ObjectGetOptions^  getOptions);
        public: BcdObject(System::Management::ManagementScope^  mgmtScope, System::Management::ManagementPath^  path);
        public: BcdObject(System::Management::ManagementPath^  path);
        public: BcdObject(System::Management::ManagementScope^  mgmtScope, System::Management::ManagementPath^  path, System::Management::ObjectGetOptions^  getOptions);
        public: BcdObject(System::Management::ManagementObject^  theObject);
        public: BcdObject(System::Management::ManagementBaseObject^  theObject);
        // Property returns the namespace of the WMI class.
        public: [Browsable(true), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property string  OriginatingNamespace {
            string  get();
        }
        
        public: [Browsable(true), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property string  ManagementClassName {
            string  get();
        }
        
        // Property pointing to an embedded object to get System properties of the WMI object.
        public: [Browsable(true), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property ROOT::WMI::BcdObject::ManagementSystemProperties^  SystemProperties {
            ROOT::WMI::BcdObject::ManagementSystemProperties^  get();
        }
        
        // Property returning the underlying lateBound object.
        public: [Browsable(false), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property System::Management::ManagementBaseObject^  LateBoundObject {
            System::Management::ManagementBaseObject^  get();
        }
        
        // ManagementScope of the object.
        public: [Browsable(true), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property System::Management::ManagementScope^  Scope {
            System::Management::ManagementScope^  get();
            void set(System::Management::ManagementScope^  value);
        }
        
        // Property to show the commit behavior for the WMI object. If true, WMI object will be automatically saved after each property modification.(ie. Put() is called after modification of a property).
        public: [Browsable(false), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property bool AutoCommit {
            bool get();
            void set(bool value);
        }
        
        // The ManagementPath of the underlying WMI object.
        public: [Browsable(true)]
        property System::Management::ManagementPath^  Path {
            System::Management::ManagementPath^  get();
            void set(System::Management::ManagementPath^  value);
        }
        
        // Public static scope property which is used by the various methods.
        public: [Browsable(true), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        static property System::Management::ManagementScope^  StaticScope {
            System::Management::ManagementScope^  get();
            void set(System::Management::ManagementScope^  value);
        }
        
        public: [Browsable(true), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property string  Id {
            string  get();
        }
        
        public: [Browsable(true), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property string  StoreFilePath {
            string  get();
        }
        
        public: [Browsable(false), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden)]
        property bool IsTypeNull {
            bool get();
        }
        
        /*public: [Browsable(true), 
        DesignerSerializationVisibility(System::ComponentModel::DesignerSerializationVisibility::Hidden),  
        TypeConverter(ROOT::WMI::BcdObject::WMIValueTypeConverter::typeid)]
        property uint Type {
            uint get();
        }*/
        
        private: bool CheckIfProperClass(System::Management::ManagementScope^  mgmtScope, System::Management::ManagementPath^  path, 
                    System::Management::ObjectGetOptions^  OptionsParam);
        
        private: bool CheckIfProperClass(System::Management::ManagementBaseObject^  theObj);
        
        private: bool ShouldSerializeType();
        
        public: [Browsable(true)]
        void CommitObject();
        
        public: [Browsable(true)]
        void CommitObject(System::Management::PutOptions^  putOptions);
        
        private: void Initialize();
        
        private: static string  ConstructPath(string  keyId, string  keyStoreFilePath);
        
        private: void InitializeObject(System::Management::ManagementScope^  mgmtScope, System::Management::ManagementPath^  path, 
                    System::Management::ObjectGetOptions^  getOptions);
        
        // Different overloads of GetInstances() help in enumerating instances of the WMI class.
        public: static ROOT::WMI::BcdObject::BcdObjectCollection^  GetInstances();
        
        public: static ROOT::WMI::BcdObject::BcdObjectCollection^  GetInstances(string  condition);
        
        public: static ROOT::WMI::BcdObject::BcdObjectCollection^  GetInstances(cli::array< string   >^  selectedProperties);
        
        public: static ROOT::WMI::BcdObject::BcdObjectCollection^  GetInstances(string  condition, cli::array< string   >^  selectedProperties);
        
        public: static ROOT::WMI::BcdObject::BcdObjectCollection^  GetInstances(System::Management::ManagementScope^  mgmtScope, 
                    System::Management::EnumerationOptions^  enumOptions);
        
        public: static ROOT::WMI::BcdObject::BcdObjectCollection^  GetInstances(System::Management::ManagementScope^  mgmtScope, 
                    string  condition);
        
        public: static ROOT::WMI::BcdObject::BcdObjectCollection^  GetInstances(System::Management::ManagementScope^  mgmtScope, 
                    cli::array< string   >^  selectedProperties);
        
        public: static ROOT::WMI::BcdObject::BcdObjectCollection^  GetInstances(System::Management::ManagementScope^  mgmtScope, 
                    string  condition, cli::array< string   >^  selectedProperties);
        
        public: [Browsable(true)]
        static ROOT::WMI::BcdObject^  CreateInstance();
        
        public: [Browsable(true)]
        void Delete();
        
        public: bool DeleteElement(uint Type);
        
        public: bool EnumerateElements(cli::array< System::Management::ManagementBaseObject^  >^  %Elements);
        
        public: bool EnumerateElementTypes(cli::array< uint >^  %Types);
        
        public: bool GetElement(uint Type, System::Management::ManagementBaseObject^  %Element);
        
        public: bool SetBooleanElement(bool Boolean, uint Type);
        
        public: bool SetDeviceElement(string  AdditionalOptions, uint DeviceType, uint Type);
        
        public: bool SetFileDeviceElement(
                    string  AdditionalOptions, 
                    uint DeviceType, 
                    string  ParentAdditionalOptions, 
                    uint ParentDeviceType, 
                    string  ParentPath, 
                    string  Path, 
                    uint Type);
        
        public: bool SetIntegerElement(System::UInt64 Integer, uint Type);
        
        public: bool SetIntegerListElement(cli::array< System::UInt64 >^  Integers, uint Type);
        
        public: bool SetObjectElement(string  Id, uint Type);
        
        public: bool SetObjectListElement(cli::array< string  >^  Ids, uint Type);
        
        public: bool SetPartitionDeviceElement(string  AdditionalOptions, uint DeviceType, 
                    string  Path, uint Type);
        
        public: bool SetStringElement(string  String, uint Type);
        
        public : // Enumerator implementation for enumerating instances of the class.
        ref class BcdObjectCollection : public System::Object, public System::Collections::ICollection {
            public : ref class BcdObjectEnumerator;
            
            private: ManagementObjectCollection^  privColObj;
            
            public: BcdObjectCollection(ManagementObjectCollection^  objCollection);
            public: virtual property int Count {
                int get();
            }
            
            public: virtual property bool IsSynchronized {
                bool get();
            }
            
            public: virtual property object  SyncRoot {
                object  get();
            }
            
            public: virtual void CopyTo(System::Array^  array, int index);
            
            public: virtual System::Collections::IEnumerator^  GetEnumerator();
            
            public : ref class BcdObjectEnumerator : public System::Object, public System::Collections::IEnumerator {
                
                private: ManagementObjectCollection::ManagementObjectEnumerator^  privObjEnum;
                
                public: BcdObjectEnumerator(ManagementObjectCollection::ManagementObjectEnumerator^  objEnum);
                public: virtual property object  Current {
                    object  get();
                }
                
                public: virtual bool MoveNext();
                
                public: virtual void Reset();
            };
        };
        
        /*public : // TypeConverter to handle null values for ValueType properties
        ref class WMIValueTypeConverter : public System::ComponentModel::TypeConverter {
            
            private: System::ComponentModel::TypeConverter^  baseConverter;
            
            private: System::Type^  baseType;
            
            public: WMIValueTypeConverter(System::Type^  inBaseType);
            public: virtual bool CanConvertFrom(System::ComponentModel::ITypeDescriptorContext^  context, System::Type^  srcType) override;
            
            public: virtual bool CanConvertTo(System::ComponentModel::ITypeDescriptorContext^  context, System::Type^  destinationType) override;
            
            public: virtual object  ConvertFrom(System::ComponentModel::ITypeDescriptorContext^  context, System::Globalization::CultureInfo^  culture, 
                        object  __identifier(value)) override;
            
            public: virtual object  CreateInstance(System::ComponentModel::ITypeDescriptorContext^  context, System::Collections::IDictionary^  dictionary) override;
            
            public: virtual bool GetCreateInstanceSupported(System::ComponentModel::ITypeDescriptorContext^  context) override;
            
            public: virtual System::ComponentModel::PropertyDescriptorCollection^  GetProperties(System::ComponentModel::ITypeDescriptorContext^  context, 
                        object  __identifier(value), cli::array< System::Attribute^  >^  attributeVar) override;
            
            public: virtual bool GetPropertiesSupported(System::ComponentModel::ITypeDescriptorContext^  context) override;
            
            public: virtual System::ComponentModel::TypeConverter::StandardValuesCollection^  GetStandardValues(System::ComponentModel::ITypeDescriptorContext^  context) override;
            
            public: virtual bool GetStandardValuesExclusive(System::ComponentModel::ITypeDescriptorContext^  context) override;
            
            public: virtual bool GetStandardValuesSupported(System::ComponentModel::ITypeDescriptorContext^  context) override;
            
            public: virtual object  ConvertTo(System::ComponentModel::ITypeDescriptorContext^  context, System::Globalization::CultureInfo^  culture, 
                        object  __identifier(value), System::Type^  destinationType) override;
        };*/
        
        public : // Embedded class to represent WMI system Properties.
        [TypeConverter(System::ComponentModel::ExpandableObjectConverter::typeid)]
        ref class ManagementSystemProperties {
            
            private: System::Management::ManagementBaseObject^  PrivateLateBoundObject;
            
            public: ManagementSystemProperties(System::Management::ManagementBaseObject^  ManagedObject);
            public: [Browsable(true)]
            property int GENUS {
                int get();
            }
            
            public: [Browsable(true)]
            property string  CLASS {
                string  get();
            }
            
            public: [Browsable(true)]
            property string  SUPERCLASS {
                string  get();
            }
            
            public: [Browsable(true)]
            property string  DYNASTY {
                string  get();
            }
            
            public: [Browsable(true)]
            property string  RELPATH {
                string  get();
            }
            
            public: [Browsable(true)]
            property int PROPERTY_COUNT {
                int get();
            }
            
            public: [Browsable(true)]
            property cli::array< string  >^  DERIVATION {
                cli::array< string  >^  get();
            }
            
            public: [Browsable(true)]
            property string  SERVER {
                string  get();
            }
            
            public: [Browsable(true)]
            property string  NAMESPACE {
                string  get();
            }
            
            public: [Browsable(true)]
            property string  PATH {
                string  get();
            }
        };
    };
    }
}
namespace ROOT {
    namespace WMI {
    
    
    inline BcdObject::BcdObject() {
        this->InitializeObject(nullptr, nullptr, nullptr);
    }
    
    inline BcdObject::BcdObject(string  keyId, string  keyStoreFilePath) {
        this->InitializeObject(nullptr, (gcnew System::Management::ManagementPath(ROOT::WMI::BcdObject::ConstructPath(keyId, 
                    keyStoreFilePath))), nullptr);
    }
    
    inline BcdObject::BcdObject(System::Management::ManagementScope^  mgmtScope, string  keyId, string  keyStoreFilePath) {
        this->InitializeObject((cli::safe_cast<System::Management::ManagementScope^  >(mgmtScope)), (gcnew System::Management::ManagementPath(ROOT::WMI::BcdObject::ConstructPath(keyId, 
                    keyStoreFilePath))), nullptr);
    }
    
    inline BcdObject::BcdObject(System::Management::ManagementPath^  path, System::Management::ObjectGetOptions^  getOptions) {
        this->InitializeObject(nullptr, path, getOptions);
    }
    
    inline BcdObject::BcdObject(System::Management::ManagementScope^  mgmtScope, System::Management::ManagementPath^  path) {
        this->InitializeObject(mgmtScope, path, nullptr);
    }
    
    inline BcdObject::BcdObject(System::Management::ManagementPath^  path) {
        this->InitializeObject(nullptr, path, nullptr);
    }
    
    inline BcdObject::BcdObject(System::Management::ManagementScope^  mgmtScope, System::Management::ManagementPath^  path, 
                System::Management::ObjectGetOptions^  getOptions) {
        this->InitializeObject(mgmtScope, path, getOptions);
    }
    
    inline BcdObject::BcdObject(System::Management::ManagementObject^  theObject) {
        Initialize();
        if (CheckIfProperClass(theObject) == true) {
            PrivateLateBoundObject = theObject;
            PrivateSystemProperties = (gcnew ROOT::WMI::BcdObject::ManagementSystemProperties(PrivateLateBoundObject));
            curObj = PrivateLateBoundObject;
        }
        else {
            throw (gcnew System::ArgumentException(L"Class name does not match."));
        }
    }
    
    inline BcdObject::BcdObject(System::Management::ManagementBaseObject^  theObject) {
        Initialize();
        if (CheckIfProperClass(theObject) == true) {
            embeddedObj = theObject;
            PrivateSystemProperties = (gcnew ROOT::WMI::BcdObject::ManagementSystemProperties(theObject));
            curObj = embeddedObj;
            isEmbedded = true;
        }
        else {
            throw (gcnew System::ArgumentException(L"Class name does not match."));
        }
    }
    
    inline string  BcdObject::OriginatingNamespace::get() {
        return L"root\\WMI";
    }
    
    inline string  BcdObject::ManagementClassName::get() {
        string  strRet = CreatedClassName;
        if (curObj != nullptr) {
            if (curObj->ClassPath != nullptr) {
                strRet = (cli::safe_cast<string  >(curObj[L"__CLASS"]));
                if ((strRet == nullptr) 
                            || (strRet == System::String::Empty)) {
                    strRet = CreatedClassName;
                }
            }
        }
        return strRet;
    }
    
    inline ROOT::WMI::BcdObject::ManagementSystemProperties^  BcdObject::SystemProperties::get() {
        return PrivateSystemProperties;
    }
    
    inline System::Management::ManagementBaseObject^  BcdObject::LateBoundObject::get() {
        return curObj;
    }
    
    inline System::Management::ManagementScope^  BcdObject::Scope::get() {
        if (isEmbedded == false) {
            return PrivateLateBoundObject->Scope;
        }
        else {
            return nullptr;
        }
    }
    inline void BcdObject::Scope::set(System::Management::ManagementScope^  value) {
        if (isEmbedded == false) {
            PrivateLateBoundObject->Scope = value;
        }
    }
    
    inline bool BcdObject::AutoCommit::get() {
        return AutoCommitProp;
    }
    inline void BcdObject::AutoCommit::set(bool value) {
        AutoCommitProp = value;
    }
    
    inline System::Management::ManagementPath^  BcdObject::Path::get() {
        if (isEmbedded == false) {
            return PrivateLateBoundObject->Path;
        }
        else {
            return nullptr;
        }
    }
    inline void BcdObject::Path::set(System::Management::ManagementPath^  value) {
        if (isEmbedded == false) {
            if (CheckIfProperClass(nullptr, __identifier(value), nullptr) != true) {
                throw (gcnew System::ArgumentException(L"Class name does not match."));
            }
            PrivateLateBoundObject->Path = value;
        }
    }
    
    inline System::Management::ManagementScope^  BcdObject::StaticScope::get() {
        return statMgmtScope;
    }
    inline void BcdObject::StaticScope::set(System::Management::ManagementScope^  value) {
        statMgmtScope = value;
    }
    
    inline string  BcdObject::Id::get() {
        return (cli::safe_cast<string  >(curObj[L"Id"]));
    }
    
    inline string  BcdObject::StoreFilePath::get() {
        return (cli::safe_cast<string  >(curObj[L"StoreFilePath"]));
    }
    
    inline bool BcdObject::IsTypeNull::get() {
        if (curObj[L"Type"] == nullptr) {
            return true;
        }
        else {
            return false;
        }
    }
    
    /*inline uint BcdObject::Type::get() {
        if (curObj[L"Type"] == nullptr) {
            return System::Convert::ToUInt32(0);
        }
        return (cli::safe_cast<uint >(curObj[L"Type"]));
    }*/
    
    inline bool BcdObject::CheckIfProperClass(System::Management::ManagementScope^  mgmtScope, System::Management::ManagementPath^  path, 
                System::Management::ObjectGetOptions^  OptionsParam) {
        if ((path != nullptr) 
                    && (System::String::Compare(path->ClassName, this->ManagementClassName, true, System::Globalization::CultureInfo::InvariantCulture) == 0)) {
            return true;
        }
        else {
            return CheckIfProperClass((gcnew System::Management::ManagementObject(mgmtScope, path, OptionsParam)));
        }
    }
    
    inline bool BcdObject::CheckIfProperClass(System::Management::ManagementBaseObject^  theObj) {
        if ((theObj != nullptr) 
                    && (System::String::Compare((cli::safe_cast<string  >(theObj[L"__CLASS"])), this->ManagementClassName, 
                        true, System::Globalization::CultureInfo::InvariantCulture) == 0)) {
            return true;
        }
        else {
            System::Array^  parentClasses = (cli::safe_cast<System::Array^  >(theObj[L"__DERIVATION"]));
            if (parentClasses != nullptr) {
                int count = 0;
                for (                count = 0; (count < parentClasses->Length);                 count = (count + 1)) {
                    if (System::String::Compare((cli::safe_cast<string  >(parentClasses->GetValue(count))), this->ManagementClassName, 
                        true, System::Globalization::CultureInfo::InvariantCulture) == 0) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    
    inline bool BcdObject::ShouldSerializeType() {
        if (this->IsTypeNull == false) {
            return true;
        }
        return false;
    }
    
    inline void BcdObject::CommitObject() {
        if (isEmbedded == false) {
            PrivateLateBoundObject->Put();
        }
    }
    
    inline void BcdObject::CommitObject(System::Management::PutOptions^  putOptions) {
        if (isEmbedded == false) {
            PrivateLateBoundObject->Put(putOptions);
        }
    }
    
    inline void BcdObject::Initialize() {
        AutoCommitProp = true;
        isEmbedded = false;
    }
    
    inline string  BcdObject::ConstructPath(string  keyId, string  keyStoreFilePath) {
        string  strPath = L"root\\WMI:BcdObject";
        strPath = System::String::Concat(strPath, System::String::Concat(L".Id=", System::String::Concat(L"\"", System::String::Concat(keyId, 
                        L"\""))));
        strPath = System::String::Concat(strPath, System::String::Concat(L",StoreFilePath=", System::String::Concat(L"\"", 
                    System::String::Concat(keyStoreFilePath, L"\""))));
        return strPath;
    }
    
    inline void BcdObject::InitializeObject(System::Management::ManagementScope^  mgmtScope, System::Management::ManagementPath^  path, 
                System::Management::ObjectGetOptions^  getOptions) {
        Initialize();
        if (path != nullptr) {
            if (CheckIfProperClass(mgmtScope, path, getOptions) != true) {
                throw (gcnew System::ArgumentException(L"Class name does not match."));
            }
        }
        PrivateLateBoundObject = (gcnew System::Management::ManagementObject(mgmtScope, path, getOptions));
        PrivateSystemProperties = (gcnew ROOT::WMI::BcdObject::ManagementSystemProperties(PrivateLateBoundObject));
        curObj = PrivateLateBoundObject;
    }
    
    inline ROOT::WMI::BcdObject::BcdObjectCollection^  BcdObject::GetInstances() {
        return GetInstances(nullptr, nullptr, nullptr);
    }
    
    inline ROOT::WMI::BcdObject::BcdObjectCollection^  BcdObject::GetInstances(string  condition) {
        return GetInstances(nullptr, condition, nullptr);
    }
    
    inline ROOT::WMI::BcdObject::BcdObjectCollection^  BcdObject::GetInstances(cli::array< string   >^  selectedProperties) {
        return GetInstances(nullptr, nullptr, selectedProperties);
    }
    
    inline ROOT::WMI::BcdObject::BcdObjectCollection^  BcdObject::GetInstances(string  condition, cli::array< string   >^  selectedProperties) {
        return GetInstances(nullptr, condition, selectedProperties);
    }
    
    inline ROOT::WMI::BcdObject::BcdObjectCollection^  BcdObject::GetInstances(System::Management::ManagementScope^  mgmtScope, 
                System::Management::EnumerationOptions^  enumOptions) {
        if (mgmtScope == nullptr) {
            if (statMgmtScope == nullptr) {
                mgmtScope = (gcnew System::Management::ManagementScope());
                mgmtScope->Path->NamespacePath = L"root\\WMI";
            }
            else {
                mgmtScope = statMgmtScope;
            }
        }
        System::Management::ManagementPath^  pathObj = (gcnew System::Management::ManagementPath());
        pathObj->ClassName = L"BcdObject";
        pathObj->NamespacePath = L"root\\WMI";
        System::Management::ManagementClass^  clsObject = (gcnew System::Management::ManagementClass(mgmtScope, pathObj, nullptr));
        if (enumOptions == nullptr) {
            enumOptions = (gcnew System::Management::EnumerationOptions());
            enumOptions->EnsureLocatable = true;
        }
        return (gcnew ROOT::WMI::BcdObject::BcdObjectCollection(clsObject->GetInstances(enumOptions)));
    }
    
    inline ROOT::WMI::BcdObject::BcdObjectCollection^  BcdObject::GetInstances(System::Management::ManagementScope^  mgmtScope, 
                string  condition) {
        return GetInstances(mgmtScope, condition, nullptr);
    }
    
    inline ROOT::WMI::BcdObject::BcdObjectCollection^  BcdObject::GetInstances(System::Management::ManagementScope^  mgmtScope, 
                cli::array< string   >^  selectedProperties) {
        return GetInstances(mgmtScope, nullptr, selectedProperties);
    }
    
    inline ROOT::WMI::BcdObject::BcdObjectCollection^  BcdObject::GetInstances(System::Management::ManagementScope^  mgmtScope, 
                string  condition, cli::array< string   >^  selectedProperties) {
        if (mgmtScope == nullptr) {
            if (statMgmtScope == nullptr) {
                mgmtScope = (gcnew System::Management::ManagementScope());
                mgmtScope->Path->NamespacePath = L"root\\WMI";
            }
            else {
                mgmtScope = statMgmtScope;
            }
        }
        System::Management::ManagementObjectSearcher^  ObjectSearcher = (gcnew System::Management::ManagementObjectSearcher(mgmtScope, 
            (gcnew SelectQuery(L"BcdObject", condition, selectedProperties))));
        System::Management::EnumerationOptions^  enumOptions = (gcnew System::Management::EnumerationOptions());
        enumOptions->EnsureLocatable = true;
        ObjectSearcher->Options = enumOptions;
        return (gcnew ROOT::WMI::BcdObject::BcdObjectCollection(ObjectSearcher->Get()));
    }
    
    inline ROOT::WMI::BcdObject^  BcdObject::CreateInstance() {
        System::Management::ManagementScope^  mgmtScope = nullptr;
        if (statMgmtScope == nullptr) {
            mgmtScope = (gcnew System::Management::ManagementScope());
            mgmtScope->Path->NamespacePath = CreatedWmiNamespace;
        }
        else {
            mgmtScope = statMgmtScope;
        }
        System::Management::ManagementPath^  mgmtPath = (gcnew System::Management::ManagementPath(CreatedClassName));
        System::Management::ManagementClass^  tmpMgmtClass = (gcnew System::Management::ManagementClass(mgmtScope, mgmtPath, nullptr));
        return (gcnew ROOT::WMI::BcdObject(tmpMgmtClass->CreateInstance()));
    }
    
    inline void BcdObject::Delete() {
        PrivateLateBoundObject->Delete();
    }
    
    inline bool BcdObject::DeleteElement(uint Type) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"DeleteElement");
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"DeleteElement", inParams, 
                nullptr);
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::EnumerateElements(cli::array< System::Management::ManagementBaseObject^  >^  %Elements) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"EnumerateElements", inParams, 
                nullptr);
            Elements = (cli::safe_cast<cli::array< System::Management::ManagementBaseObject^  >^  >(outParams->Properties[L"Elements"]->Value));
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            Elements = nullptr;
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::EnumerateElementTypes(cli::array< uint >^  %Types) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"EnumerateElementTypes", 
                inParams, nullptr);
            Types = (cli::safe_cast<cli::array< uint >^  >(outParams->Properties[L"Types"]->Value));
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            Types = nullptr;
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::GetElement(uint Type, System::Management::ManagementBaseObject^  %Element) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"GetElement");
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"GetElement", inParams, 
                nullptr);
            Element = (cli::safe_cast<System::Management::ManagementBaseObject^  >(outParams->Properties[L"Element"]->Value));
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            Element = nullptr;
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::SetBooleanElement(bool Boolean, uint Type) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"SetBooleanElement");
            inParams[L"Boolean"] = (cli::safe_cast<bool ^  >(Boolean));
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"SetBooleanElement", inParams, 
                nullptr);
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::SetDeviceElement(string  AdditionalOptions, uint DeviceType, uint Type) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"SetDeviceElement");
            inParams[L"AdditionalOptions"] = (cli::safe_cast<string   >(AdditionalOptions));
            inParams[L"DeviceType"] = (cli::safe_cast<uint ^  >(DeviceType));
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"SetDeviceElement", inParams, 
                nullptr);
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::SetFileDeviceElement(
                string  AdditionalOptions, 
                uint DeviceType, 
                string  ParentAdditionalOptions, 
                uint ParentDeviceType, 
                string  ParentPath, 
                string  Path, 
                uint Type) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"SetFileDeviceElement");
            inParams[L"AdditionalOptions"] = (cli::safe_cast<string   >(AdditionalOptions));
            inParams[L"DeviceType"] = (cli::safe_cast<uint ^  >(DeviceType));
            inParams[L"ParentAdditionalOptions"] = (cli::safe_cast<string   >(ParentAdditionalOptions));
            inParams[L"ParentDeviceType"] = (cli::safe_cast<uint ^  >(ParentDeviceType));
            inParams[L"ParentPath"] = (cli::safe_cast<string   >(ParentPath));
            inParams[L"Path"] = (cli::safe_cast<string   >(Path));
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"SetFileDeviceElement", 
                inParams, nullptr);
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::SetIntegerElement(System::UInt64 Integer, uint Type) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"SetIntegerElement");
            inParams[L"Integer"] = (cli::safe_cast<System::UInt64 ^  >(Integer));
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"SetIntegerElement", inParams, 
                nullptr);
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::SetIntegerListElement(cli::array< System::UInt64 >^  Integers, uint Type) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"SetIntegerListElement");
            inParams[L"Integers"] = (cli::safe_cast<cli::array< System::UInt64 >^  >(Integers));
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"SetIntegerListElement", 
                inParams, nullptr);
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::SetObjectElement(string  Id, uint Type) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"SetObjectElement");
            inParams[L"Id"] = (cli::safe_cast<string   >(Id));
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"SetObjectElement", inParams, 
                nullptr);
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::SetObjectListElement(cli::array< string  >^  Ids, uint Type) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"SetObjectListElement");
            inParams[L"Ids"] = (cli::safe_cast<cli::array< string  >^  >(Ids));
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"SetObjectListElement", 
                inParams, nullptr);
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::SetPartitionDeviceElement(string  AdditionalOptions, uint DeviceType, 
                string  Path, uint Type) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"SetPartitionDeviceElement");
            inParams[L"AdditionalOptions"] = (cli::safe_cast<string   >(AdditionalOptions));
            inParams[L"DeviceType"] = (cli::safe_cast<uint ^  >(DeviceType));
            inParams[L"Path"] = (cli::safe_cast<string   >(Path));
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"SetPartitionDeviceElement", 
                inParams, nullptr);
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            return System::Convert::ToBoolean(0);
        }
    }
    
    inline bool BcdObject::SetStringElement(string  String, uint Type) {
        if (isEmbedded == false) {
            System::Management::ManagementBaseObject^  inParams = nullptr;
            inParams = PrivateLateBoundObject->GetMethodParameters(L"SetStringElement");
            inParams[L"String"] = (cli::safe_cast<string   >(String));
            inParams[L"Type"] = (cli::safe_cast<uint ^  >(Type));
            System::Management::ManagementBaseObject^  outParams = PrivateLateBoundObject->InvokeMethod(L"SetStringElement", inParams, 
                nullptr);
            return System::Convert::ToBoolean(outParams->Properties[L"ReturnValue"]->Value);
        }
        else {
            return System::Convert::ToBoolean(0);
        }
    }
    
    
    inline BcdObject::BcdObjectCollection::BcdObjectCollection(ManagementObjectCollection^  objCollection) {
        privColObj = objCollection;
    }
    
    inline int BcdObject::BcdObjectCollection::Count::get() {
        return privColObj->Count;
    }
    
    inline bool BcdObject::BcdObjectCollection::IsSynchronized::get() {
        return privColObj->IsSynchronized;
    }
    
    inline object  BcdObject::BcdObjectCollection::SyncRoot::get() {
        return this;
    }
    
    inline void BcdObject::BcdObjectCollection::CopyTo(System::Array^  array, int index) {
        privColObj->CopyTo(array, index);
        int nCtr;
        for (        nCtr = 0; (nCtr < array->Length);         nCtr = (nCtr + 1)) {
            array->SetValue((gcnew ROOT::WMI::BcdObject((cli::safe_cast<System::Management::ManagementObject^  >(array->GetValue(nCtr))))), 
                nCtr);
        }
    }
    
    inline System::Collections::IEnumerator^  BcdObject::BcdObjectCollection::GetEnumerator() {
        return (gcnew ROOT::WMI::BcdObject::BcdObjectCollection::BcdObjectEnumerator(privColObj->GetEnumerator()));
    }
    
    
    inline BcdObject::BcdObjectCollection::BcdObjectEnumerator::BcdObjectEnumerator(ManagementObjectCollection::ManagementObjectEnumerator^  objEnum) {
        privObjEnum = objEnum;
    }
    
    inline object  BcdObject::BcdObjectCollection::BcdObjectEnumerator::Current::get() {
        return (gcnew ROOT::WMI::BcdObject((cli::safe_cast<System::Management::ManagementObject^  >(privObjEnum->Current))));
    }
    
    inline bool BcdObject::BcdObjectCollection::BcdObjectEnumerator::MoveNext() {
        return privObjEnum->MoveNext();
    }
    
    inline void BcdObject::BcdObjectCollection::BcdObjectEnumerator::Reset() {
        privObjEnum->Reset();
    }
    
    
    /*inline BcdObject::WMIValueTypeConverter::WMIValueTypeConverter(System::Type^  inBaseType) {
        baseConverter = System::ComponentModel::TypeDescriptor::GetConverter(inBaseType);
        baseType = inBaseType;
    }
    
    inline bool BcdObject::WMIValueTypeConverter::CanConvertFrom(System::ComponentModel::ITypeDescriptorContext^  context, 
                System::Type^  srcType) {
        return baseConverter->CanConvertFrom(context, srcType);
    }
    
    inline bool BcdObject::WMIValueTypeConverter::CanConvertTo(System::ComponentModel::ITypeDescriptorContext^  context, 
                System::Type^  destinationType) {
        return baseConverter->CanConvertTo(context, destinationType);
    }
    
    inline object  BcdObject::WMIValueTypeConverter::ConvertFrom(System::ComponentModel::ITypeDescriptorContext^  context, 
                System::Globalization::CultureInfo^  culture, object  __identifier(value)) {
        return baseConverter->ConvertFrom(context, culture, __identifier(value));
    }
    
    inline object  BcdObject::WMIValueTypeConverter::CreateInstance(System::ComponentModel::ITypeDescriptorContext^  context, 
                System::Collections::IDictionary^  dictionary) {
        return baseConverter->CreateInstance(context, dictionary);
    }
    
    inline bool BcdObject::WMIValueTypeConverter::GetCreateInstanceSupported(System::ComponentModel::ITypeDescriptorContext^  context) {
        return baseConverter->GetCreateInstanceSupported(context);
    }
    
    inline System::ComponentModel::PropertyDescriptorCollection^  BcdObject::WMIValueTypeConverter::GetProperties(System::ComponentModel::ITypeDescriptorContext^  context, 
                object  __identifier(value), cli::array< System::Attribute^  >^  attributeVar) {
        return baseConverter->GetProperties(context, __identifier(value), attributeVar);
    }
    
    inline bool BcdObject::WMIValueTypeConverter::GetPropertiesSupported(System::ComponentModel::ITypeDescriptorContext^  context) {
        return baseConverter->GetPropertiesSupported(context);
    }
    
    inline System::ComponentModel::TypeConverter::StandardValuesCollection^  BcdObject::WMIValueTypeConverter::GetStandardValues(
                System::ComponentModel::ITypeDescriptorContext^  context) {
        return baseConverter->GetStandardValues(context);
    }
    
    inline bool BcdObject::WMIValueTypeConverter::GetStandardValuesExclusive(System::ComponentModel::ITypeDescriptorContext^  context) {
        return baseConverter->GetStandardValuesExclusive(context);
    }
    
    inline bool BcdObject::WMIValueTypeConverter::GetStandardValuesSupported(System::ComponentModel::ITypeDescriptorContext^  context) {
        return baseConverter->GetStandardValuesSupported(context);
    }
    
    inline object  BcdObject::WMIValueTypeConverter::ConvertTo(System::ComponentModel::ITypeDescriptorContext^  context, 
                System::Globalization::CultureInfo^  culture, object  __identifier(value), System::Type^  destinationType) {
        if (baseType->BaseType == System::Enum::typeid) {
            if (__identifier(value)->GetType() == destinationType) {
                return __identifier(value);
            }
            if (((__identifier(value) == nullptr) 
                        && (context != nullptr)) 
                        && (context->PropertyDescriptor->ShouldSerializeValue(context->Instance) == false)) {
                return  "NULL_ENUM_VALUE" ;
            }
            return baseConverter->ConvertTo(context, culture, __identifier(value), destinationType);
        }
        if ((baseType == bool::typeid) 
                    && (baseType->BaseType == System::ValueType::typeid)) {
            if (((__identifier(value) == nullptr) 
                        && (context != nullptr)) 
                        && (context->PropertyDescriptor->ShouldSerializeValue(context->Instance) == false)) {
                return L"";
            }
            return baseConverter->ConvertTo(context, culture, __identifier(value), destinationType);
        }
        if ((context != nullptr) 
                    && (context->PropertyDescriptor->ShouldSerializeValue(context->Instance) == false)) {
            return L"";
        }
        return baseConverter->ConvertTo(context, culture, __identifier(value), destinationType);
    }*/
    
    
    inline BcdObject::ManagementSystemProperties::ManagementSystemProperties(System::Management::ManagementBaseObject^  ManagedObject) {
        PrivateLateBoundObject = ManagedObject;
    }
    
    inline int BcdObject::ManagementSystemProperties::GENUS::get() {
        return (cli::safe_cast<int >(PrivateLateBoundObject[L"__GENUS"]));
    }
    
    inline string  BcdObject::ManagementSystemProperties::CLASS::get() {
        return (cli::safe_cast<string  >(PrivateLateBoundObject[L"__CLASS"]));
    }
    
    inline string  BcdObject::ManagementSystemProperties::SUPERCLASS::get() {
        return (cli::safe_cast<string  >(PrivateLateBoundObject[L"__SUPERCLASS"]));
    }
    
    inline string  BcdObject::ManagementSystemProperties::DYNASTY::get() {
        return (cli::safe_cast<string  >(PrivateLateBoundObject[L"__DYNASTY"]));
    }
    
    inline string  BcdObject::ManagementSystemProperties::RELPATH::get() {
        return (cli::safe_cast<string  >(PrivateLateBoundObject[L"__RELPATH"]));
    }
    
    inline int BcdObject::ManagementSystemProperties::PROPERTY_COUNT::get() {
        return (cli::safe_cast<int >(PrivateLateBoundObject[L"__PROPERTY_COUNT"]));
    }
    
    inline cli::array< string  >^  BcdObject::ManagementSystemProperties::DERIVATION::get() {
        return (cli::safe_cast<cli::array< string  >^  >(PrivateLateBoundObject[L"__DERIVATION"]));
    }
    
    inline string  BcdObject::ManagementSystemProperties::SERVER::get() {
        return (cli::safe_cast<string  >(PrivateLateBoundObject[L"__SERVER"]));
    }
    
    inline string  BcdObject::ManagementSystemProperties::NAMESPACE::get() {
        return (cli::safe_cast<string  >(PrivateLateBoundObject[L"__NAMESPACE"]));
    }
    
    inline string  BcdObject::ManagementSystemProperties::PATH::get() {
        return (cli::safe_cast<string  >(PrivateLateBoundObject[L"__PATH"]));
    }
    }
}
