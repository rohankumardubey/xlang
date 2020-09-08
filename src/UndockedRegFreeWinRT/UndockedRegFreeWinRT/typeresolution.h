#pragma once

#include <RoMetadataApi.h>
#include <hstring.h>
#include <windows.foundation.h>
#include <Windows.h>
#include <wil/result.h>
#include <wil/resource.h>
#include <wil/filesystem.h>
#include <wrl.h>
#include <Synchapi.h>
#include <unordered_map>

namespace UndockedRegFreeWinRT
{
    typedef enum
    {
        TRO_NONE = 0x00,
        TRO_RESOLVE_TYPE = 0x01,
        TRO_RESOLVE_NAMESPACE = 0x02,
        TRO_RESOLVE_TYPE_AND_NAMESPACE = TRO_RESOLVE_TYPE | TRO_RESOLVE_NAMESPACE
    } TYPE_RESOLUTION_OPTIONS;

    // Returned string is cached globally, and should not be freed by the caller.
    HRESULT GetProcessExeDir(PCWSTR* path);

    HRESULT FindTypeInMetaDataFile(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __in PCWSTR pszCandidateFilePath,
        __in TYPE_RESOLUTION_OPTIONS resolutionOptions,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef);

    HRESULT FindTypeInDirectory(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __in PCWSTR pszDirectoryPath,
        __out_opt HSTRING* phstrMetaDataFilePath,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef);

    HRESULT FindTypeInDirectoryWithNormalization(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __in PCWSTR pszDirectoryPath,
        __out_opt HSTRING* phstrMetaDataFilePath,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef);

    HRESULT ResolveThirdPartyType(
        __in IMetaDataDispenserEx* pMetaDataDispenser,
        __in PCWSTR pszFullName,
        __out_opt HSTRING* phstrMetaDataFilePath,
        __deref_opt_out_opt IMetaDataImport2** ppMetaDataImport,
        __out_opt mdTypeDef* pmdTypeDef);

    inline PWSTR AllocateAndCopyString(_In_ PCWSTR pszKey)
    {
        size_t length = wcslen(pszKey);
        PWSTR pszKeyCopy = nullptr;

        if (length <= MAX_PATH)
        {
            pszKeyCopy = new WCHAR[length + 1];
            if (pszKeyCopy != nullptr)
            {
                SUCCEEDED(StringCchCopyW(pszKeyCopy, length + 1, pszKey));
            }
        }

        return pszKeyCopy;
    }

    //
    // Metada importers LRU cache. Singleton.
    //
    const DWORD g_dwMetaDataImportersLRUCacheSize = 5;

    class MetaDataImportersLRUCache
    {
    public:
        static MetaDataImportersLRUCache* GetMetaDataImportersLRUCacheInstance();

        HRESULT GetMetaDataImporter(
            _In_ IMetaDataDispenserEx* pMetaDataDispenser,
            _In_ PCWSTR pszCandidateFilePath,
            _Outptr_opt_ IMetaDataImport2** ppMetaDataImporter);

    private:
        MetaDataImportersLRUCache()
        {
            InitializeCriticalSection(&_csCacheLock);
            ZeroMemory(_arFilePaths, sizeof(_arFilePaths));
        }

        ~MetaDataImportersLRUCache()
        {
            _metadataImportersMap.clear();
            for (int i = 0; i < g_dwMetaDataImportersLRUCacheSize; i++)
            {
                if (_arFilePaths[i] != nullptr)
                {
                    delete[] _arFilePaths[i];
                    _arFilePaths[i] = nullptr;
                }
            }

            DeleteCriticalSection(&_csCacheLock);
        }

        static BOOL CALLBACK ConstructLRUCacheIfNecessary(
            PINIT_ONCE /*initOnce*/,
            PVOID /*parameter*/,
            PVOID* /*context*/);

        HRESULT GetNewMetaDataImporter(
            _In_ IMetaDataDispenserEx* pMetaDataDispenser,
            _In_ PCWSTR pszCandidateFilePath,
            _Outptr_opt_ IMetaDataImport2** ppMetaDataImporter);

        bool IsFilePathCached(PCWSTR pszFilePath);
        int GetFilePathIndex(PCWSTR pszFilePath);
        void MoveElementToFrontOfList(int elementIndex);
        HRESULT AddNewFilePathToList(PCWSTR pszFilePath);
        HRESULT RemoveLeastRecentlyUsedItemIfListIsFull();

        static INIT_ONCE s_initOnce;
        static MetaDataImportersLRUCache* s_pMetaDataImportersLRUCacheInstance;
        std::unordered_map<PCWSTR, IMetaDataImport2*> _metadataImportersMap;
        PWSTR _arFilePaths[g_dwMetaDataImportersLRUCacheSize];
        CRITICAL_SECTION _csCacheLock;
    };
}