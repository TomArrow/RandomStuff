//#define SHARPZIPLIB
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
#if SHARPZIPLIB
using ICSharpCode.SharpZipLib.Zip;
#else
using System.IO.Compression;
#endif
class Program
{
    static void Main(string[] args)
    {
        string mainFolderPath = args[0];
        Regex searchFile = new Regex(args[1],RegexOptions.IgnoreCase|RegexOptions.Compiled);

        Dictionary<string,Dictionary<int, List<string>>> versionsWhere = new Dictionary<string, Dictionary<int, List<string>>>();

        Dictionary<string,List<byte[]>> versions = new Dictionary<string, List<byte[]>>();

        AnalyzeFolder(mainFolderPath, searchFile, ref versionsWhere, ref versions);


        StringBuilder sb = new StringBuilder();
        foreach (var kvp in versions)
        {
            sb.Append($"\n{kvp.Key}:\n");
            for (int i = 0; i < kvp.Value.Count; i++)
            {

                sb.Append($"Version {i} is in the following zip files:\n");
                foreach (string path in versionsWhere[kvp.Key][i])
                {
                    sb.Append($"\t{path}\n");
                }
            }
        }
        Console.WriteLine(sb.ToString());
        File.AppendAllText($"foundVersions.txt", sb.ToString());


        Console.ReadKey();
    }

    static void AnalyzeFolder(string folderPath, Regex searchFile, ref Dictionary<string, Dictionary<int, List<string>>> versionsWhere, ref Dictionary<string, List<byte[]>> versions)
    {
        string[] listOfFiles = Directory.GetFiles(folderPath);
        string[] listOfFolders = Directory.GetDirectories(folderPath);

        foreach(string file in listOfFiles)
        {
            if (file.EndsWith($".zip", StringComparison.OrdinalIgnoreCase) || file.EndsWith($".pk3", StringComparison.OrdinalIgnoreCase))
            {
                try { 
#if SHARPZIPLIB
                    using (ZipFile mainArchive = new ZipFile(file))
                    {
                        AnalyzeZipFile(Path.Combine(folderPath, file), mainArchive, searchFile, ref versionsWhere, ref versions);
                    }
#else
                    using (ZipArchive mainArchive = ZipFile.OpenRead(file))
                    {
                        AnalyzeZipFile(Path.Combine(folderPath, file), mainArchive, searchFile, ref versionsWhere, ref versions);
                    }
#endif
                } catch(Exception ex)
                {
                    Console.WriteLine($"Error opening: {file}", ex.Message);
                }
            }
        }

        foreach (string folder in listOfFolders)
        {
            AnalyzeFolder(folder, searchFile, ref versionsWhere, ref versions);
        }
    }
#if SHARPZIPLIB
    static void AnalyzeZipFile(string path, ZipFile mainArchive, Regex searchFile, ref Dictionary<string, Dictionary<int, List<string>>> versionsWhere, ref Dictionary<string, List<byte[]>> versions)
#else 
    static void AnalyzeZipFile(string path, ZipArchive mainArchive, Regex searchFile, ref Dictionary<string, Dictionary<int, List<string>>> versionsWhere, ref Dictionary<string, List<byte[]>> versions)
#endif
    {
#if SHARPZIPLIB
        foreach (ZipEntry entry in mainArchive)
#else
        foreach (ZipArchiveEntry entry in mainArchive.Entries)
#endif
        {
            if (entry.Name.EndsWith($".zip", StringComparison.OrdinalIgnoreCase) || entry.Name.EndsWith($".pk3", StringComparison.OrdinalIgnoreCase))
            {
                try
                {
#if SHARPZIPLIB
                    using (Stream zipStream = mainArchive.GetInputStream(entry))
                    {
                        using (MemoryStream ms = new MemoryStream())
                        {
                            zipStream.CopyTo(ms);
                            using (ZipFile subArchive = new ZipFile(ms))
                            {
                                AnalyzeZipFile(Path.Combine(path, entry.Name), subArchive, searchFile, ref versionsWhere, ref versions);
                            }
                        }
                    }
#else
                    using (Stream zipStream = entry.Open())
                    {
                        using (ZipArchive subArchive = new ZipArchive(zipStream))
                        {
                            AnalyzeZipFile(Path.Combine(path, entry.Name), subArchive, searchFile, ref versionsWhere, ref versions);
                        }
                    }
#endif
                } catch(Exception ex)
                {
                    Console.WriteLine($"Error opening: {path}/{entry.Name}",ex.Message);
                }
            }
            if (searchFile.Match(entry.Name).Success)
            {
#if SHARPZIPLIB
                byte[] version = ReadFileFromZipEntry(mainArchive,entry);
#else
                byte[] version = ReadFileFromZipEntry(entry);
#endif
                int indexHere = 0;
                string entryNameLower = entry.Name.ToLower();
                entryNameLower = Path.GetFileName(entryNameLower);
                if (!versions.ContainsKey(entryNameLower))
                {
                    versions[entryNameLower] = new List<byte[]>();
                }
                if (!versionsWhere.ContainsKey(entryNameLower))
                {
                    versionsWhere[entryNameLower] = new Dictionary<int, List<string>>();
                }
                if ((indexHere = findVersion(entryNameLower, ref version,ref versionsWhere, ref versions)) == -1)
                {
                    indexHere = versions[entryNameLower].Count;
                    versions[entryNameLower].Add(version);
                    versionsWhere[entryNameLower][indexHere] = new List<string>();
                    string targetFileName = entryNameLower;
                    if (indexHere != 0)
                    {
                        targetFileName = $"{Path.GetFileNameWithoutExtension(entryNameLower)}_version{(indexHere + 1)}{Path.GetExtension(entryNameLower)}";
                    }
                    File.WriteAllBytes(targetFileName, version);
                    File.SetLastWriteTime(targetFileName, entry.LastWriteTime.DateTime);
                    
                }
                versionsWhere[entryNameLower][indexHere].Add(path);
            }
        }
    }
    
    /*static void AnalyzeZipFile(string path, ZipArchive mainArchive, Regex searchFile, ref Dictionary<string, Dictionary<int, List<string>>> versionsWhere, ref Dictionary<string, List<byte[]>> versions)
    {
        foreach (ZipArchiveEntry entry in mainArchive.Entries)
        {
            if (entry.Name.EndsWith($".zip", StringComparison.OrdinalIgnoreCase) || entry.Name.EndsWith($".pk3", StringComparison.OrdinalIgnoreCase))
            {
                using (Stream zipStream = entry.Open())
                using (ZipArchive subArchive = new ZipArchive(zipStream))
                {
                    AnalyzeZipFile(Path.Combine(path,entry.Name), subArchive, searchFile, ref versionsWhere, ref versions);
                }
            }
            if (searchFile.Match(entry.Name).Success)
            {
                byte[] version = ReadFileFromZipEntry(entry);
                int indexHere = 0;
                string entryNameLower = entry.Name.ToLower();
                if (!versions.ContainsKey(entryNameLower))
                {
                    versions[entryNameLower] = new List<byte[]>();
                }
                if (!versionsWhere.ContainsKey(entryNameLower))
                {
                    versionsWhere[entryNameLower] = new Dictionary<int, List<string>>();
                }
                if ((indexHere = findVersion(entryNameLower, ref version,ref versionsWhere, ref versions)) == -1)
                {
                    indexHere = versions[entryNameLower].Count;
                    versions[entryNameLower].Add(version);
                    versionsWhere[entryNameLower][indexHere] = new List<string>();
                    if(indexHere == 0)
                    {
                        File.WriteAllBytes(entryNameLower,version);
                    }
                    else
                    {
                        File.WriteAllBytes($"{Path.GetFileNameWithoutExtension(entryNameLower)}_version{(indexHere+1)}{Path.GetExtension(entryNameLower)}", version);
                    }
                }
                versionsWhere[entryNameLower][indexHere].Add(path);
            }
        }
    }*/

    static int findVersion(string fileName,ref byte[] version, ref Dictionary<string, Dictionary<int, List<string>>> versionsWhere, ref Dictionary<string, List<byte[]>> versions)
    {
        List<byte[]> versionsHere = versions[fileName];
        for (int i=0; i< versionsHere.Count; i++ )
        {
            if (versionsHere[i].SequenceEqual(version))
            {
                return i;
            }
        }
        return -1;
    }

#if SHARPZIPLIB
    static byte[] ReadFileFromZipEntry(ZipFile mainFile, ZipEntry entry)
    {
        using (BinaryReader reader = new BinaryReader(mainFile.GetInputStream(entry)))
        {
            return reader.ReadBytes((int)entry.Size);
        }
    }
#else
    static byte[] ReadFileFromZipEntry(ZipArchiveEntry entry)
    {
        using (BinaryReader reader = new BinaryReader(entry.Open()))
        {
            return reader.ReadBytes((int)entry.Length);
        }
    }
#endif
}