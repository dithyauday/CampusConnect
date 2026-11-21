const BASE = "http://localhost:8081";

export interface Alumni {
  uid: string;
  name: string;
  department: string;
  course: string;
  graduation_year: number;
  from_year: number;
  batch: string;
  current_company: string;
  current_role: string;
  city: string;
  linkedin_url: string;
  photo_url: string;
}

export interface AlumniFilters {
  search?: string;
  batch?: string;
  department?: string;
  company?: string;
}

export async function fetchAlumni(filters: AlumniFilters = {}): Promise<Alumni[]> {
  const params = new URLSearchParams();
  if (filters.search)     params.set("search",     filters.search);
  if (filters.batch)      params.set("batch",      filters.batch);
  if (filters.department) params.set("department", filters.department);
  if (filters.company)    params.set("company",    filters.company);

  const url = `${BASE}/api/alumni${params.toString() ? "?" + params : ""}`;
  const res = await fetch(url, { cache: "no-store" });
  if (!res.ok) throw new Error(`API error ${res.status}`);
  
  let rawText = await res.text();
  console.log("RAW SERVER RESPONSE IS:", rawText); // <--- ADD THIS TEMPORARY LOG
  console.log("--- RAW FRONTEND RECEIVED DATA START ---");
  console.log(rawText.substring(0, 300)); 
  console.log("--- RAW FRONTEND RECEIVED DATA END ---");

  // 1. Clean the front: find the first opening brace '{' to skip compression artifacts
  const jsonStartIndex = rawText.indexOf('{');
  if (jsonStartIndex !== -1) {
    rawText = rawText.substring(jsonStartIndex);
  }

  // 2. Clean the back: isolate up to the very last closing object brace '}'
  const jsonEndIndex = rawText.lastIndexOf('}');
  if (jsonEndIndex !== -1) {
    rawText = rawText.substring(0, jsonEndIndex + 1);
  }

  // 3. Remove any accidental trailing commas from the end object segment
  rawText = rawText.trim();
  if (rawText.endsWith(',')) {
    rawText = rawText.slice(0, -1);
  }

  // 4. Wrap the sequence of objects into a single clean JSON array structure
  const cleanJson = `[${rawText}]`;

  console.log("--- PERFECTLY WRAPPED JSON ---");
  console.log("START:", cleanJson.substring(0, 150));
  console.log("END:", cleanJson.substring(cleanJson.length - 150));

  return JSON.parse(cleanJson);
}

export async function addAlumni(data: Omit<Alumni, "created_at">): Promise<void> {
  console.log("Sending new alumni data:", data);
  const res = await fetch(`${BASE}/api/alumni/add`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(data),
  });
  if (!res.ok) throw new Error(`API error ${res.status}`);
  console.log("Alumni added successfully!");
}